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
  
  verbly::data database {config["verbly_datafile"].as<std::string>()};
  
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
  
  for (;;)
  {
    std::cout << "Generating tweet..." << std::endl;
    
    std::string exclamation;
    while (exclamation.empty())
    {
      verbly::verb v = database.verbs().random().limit(1).has_pronunciation().run().front();
      auto rhyms = database.nouns().rhymes_with(v).random().limit(1).is_not_proper().run();
      if (!rhyms.empty())
      {
        auto n = rhyms.front();
        if (n.base_form() != v.base_form())
        {
          exclamation = "god " + v.base_form() + " this " + n.base_form();
        }
      }
    }
    
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
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }
    
    std::cout << "Waiting..." << std::endl;
    
    std::this_thread::sleep_for(std::chrono::hours(1));
  }
}
