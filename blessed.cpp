#include <yaml-cpp/yaml.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <verbly.h>
#include <twitter.h>
#include <chrono>
#include <random>
#include <thread>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: blessed [configfile]" << std::endl;
    return -1;
  }

  std::string configfile(argv[1]);

  std::random_device random_device;
  std::mt19937 random_engine{random_device()};

  YAML::Node config = YAML::LoadFile(configfile);

  twitter::auth auth;
  auth.setConsumerKey(config["consumer_key"].as<std::string>());
  auth.setConsumerSecret(config["consumer_secret"].as<std::string>());
  auth.setAccessKey(config["access_key"].as<std::string>());
  auth.setAccessSecret(config["access_secret"].as<std::string>());

  twitter::client client(auth);

  verbly::database database {config["verbly_datafile"].as<std::string>()};

  std::vector<std::string> emojis;
  {
    std::ifstream emojifile(config["emoji_file"].as<std::string>());
    if (!emojifile.is_open())
    {
      std::cout << "Could not find emoji." << std::endl;
      return -1;
    }

    std::string line;
    while (getline(emojifile, line))
    {
      if (line.back() == '\r')
      {
        line.pop_back();
      }

      emojis.push_back(line);
    }
  }

  std::uniform_int_distribution<int> emojidist(0, emojis.size()-1);
  std::bernoulli_distribution continuedist(1.0/2.0);

  verbly::query<verbly::form> verbQuery = database.forms(
    (verbly::notion::partOfSpeech == verbly::part_of_speech::verb)
    && (verbly::pronunciation::rhymes));

  for (;;)
  {
    std::cout << "Generating tweet..." << std::endl;

    verbly::form verb = verbQuery.first();

    verbly::form noun = database.forms(
      (verbly::notion::partOfSpeech == verbly::part_of_speech::noun)
      && (verbly::pronunciation::rhymes %= verb)
      && (verbly::form::proper == false)
      && (verbly::form::id != verb.getId())
      && !(verbly::word::usageDomains %= (verbly::notion::wnid == 106718862))).first();

    std::string exclamation = "god " + verb.getText() + " this " + noun.getText();

    std::string emojibef;
    std::string emojiaft;
    int emn = 0;
    do
    {
      emn++;
      std::string em = emojis[emojidist(random_engine)];
      emojibef += em;
      emojiaft = em + emojiaft;
    } while (continuedist(random_engine));

    std::string result;
    if (emn > 3)
    {
      result = emojibef + "\n" + exclamation + "\n" + emojiaft;
    } else {
      result = emojibef + " " + exclamation + " " + emojiaft;
    }

    result.resize(140);
    std::cout << result << std::endl;

    try
    {
      client.updateStatus(result);

      std::cout << "Tweeted!" << std::endl;
      std::cout << "Waiting..." << std::endl;

      std::this_thread::sleep_for(std::chrono::hours(1));

      std::cout << std::endl;
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }
  }
}
