#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <twitcurl.h>
#include <verbly.h>

int main(int argc, char** argv)
{
  srand(time(NULL));
  
  YAML::Node config = YAML::LoadFile("config.yml");
    
  twitCurl twitter;
  twitter.getOAuth().setConsumerKey(config["consumer_key"].as<std::string>());
  twitter.getOAuth().setConsumerSecret(config["consumer_secret"].as<std::string>());
  twitter.getOAuth().setOAuthTokenKey(config["access_key"].as<std::string>());
  twitter.getOAuth().setOAuthTokenSecret(config["access_secret"].as<std::string>());
  
  verbly::data database {"data.sqlite3"};
  
  std::vector<std::string> emojis;
  std::ifstream emojifile("emojis.txt");
  if (!emojifile.is_open())
  {
    std::cout << "Could not find emoji." << std::endl;
    return -1;
  }

  for (;;)
  {
    std::string line;
    if (!getline(emojifile, line))
    {
      break;
    }
  
    if (line.back() == '\r')
    {
      line.pop_back();
    }
    
    emojis.push_back(line);
  }
  
  emojifile.close();
  
  for (;;)
  {
    std::cout << "Generating tweet" << std::endl;
    
    std::string exclamation;
    for (;;)
    {
      verbly::verb v = database.verbs().random(true).limit(1).has_pronunciation(true).run().front();
      auto rhyms = database.nouns().rhymes_with(v).random(true).limit(1).not_derived_from(v).is_not_proper(true).run();
      if (!rhyms.empty())
      {
        auto n = rhyms.front();
        if (n.base_form() != v.base_form())
        {
          exclamation = "god " + v.base_form() + " this " + n.base_form();
          break;
        }
      }
    }
    
    std::string emojibef;
    std::string emojiaft;
    int emn = 0;
    do
    {
      emn++;
      std::string em = emojis[rand() % emojis.size()];
      emojibef += em;
      emojiaft = em + emojiaft;
    } while (rand() % 2 == 0);
    
    std::string result;
    if (emn > 3)
    {
      result = emojibef + "\n" + exclamation + "\n" + emojiaft;
    } else {
      result = emojibef + " " + exclamation + " " + emojiaft;
    }
    result.resize(140);
    
    std::string replyMsg;
    if (twitter.statusUpdate(result))
    {
      twitter.getLastWebResponse(replyMsg);
      std::cout << "Twitter message: " << replyMsg << std::endl;
    } else {
      twitter.getLastCurlError(replyMsg);
      std::cout << "Curl error: " << replyMsg << std::endl;
    }
    
    std::cout << "Waiting" << std::endl;
    sleep(60 * 60);
  }
}
