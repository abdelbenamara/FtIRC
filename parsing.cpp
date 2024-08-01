#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

struct Key
{
	// optional
	bool		clientSpecificTag;
	std::string host;

	// mandatory
	std::string keyName;

	bool operator<(const Key &k)  const {
		return keyName < k.keyName;
	}

	bool operator=(const Key &k)  const {
		return keyName == k.keyName;
	}
};

struct Source
{
	// optional
	std::string user;
	std::string host;

	// mandatory servername or nickname
	std::string nickname; // "" if no nickname
	std::string servername;
};

struct IRCMessage
{
	std::map<Key, std::string> tags;
	Source source;
	std::string command;
	std::vector<std::string> parameters;
};


bool is_valid_utf8(const char * string)
{
	if (!string)
		return true;

	const unsigned char * bytes = (const unsigned char *)string;
	unsigned int cp;
	int num;

	while (*bytes != 0x00)
	{
		if ((*bytes & 0x80) == 0x00)
		{
			// U+0000 to U+007F 
			cp = (*bytes & 0x7F);
			num = 1;
		}
		else if ((*bytes & 0xE0) == 0xC0)
		{
			// U+0080 to U+07FF 
			cp = (*bytes & 0x1F);
			num = 2;
		}
		else if ((*bytes & 0xF0) == 0xE0)
		{
			// U+0800 to U+FFFF 
			cp = (*bytes & 0x0F);
			num = 3;
		}
		else if ((*bytes & 0xF8) == 0xF0)
		{
			// U+10000 to U+10FFFF 
			cp = (*bytes & 0x07);
			num = 4;
		}
		else
			return false;

		bytes += 1;
		for (int i = 1; i < num; ++i)
		{
			if ((*bytes & 0xC0) != 0x80)
				return false;
			cp = (cp << 6) | (*bytes & 0x3F);
			bytes += 1;
		}

		if ((cp > 0x10FFFF) ||
			((cp >= 0xD800) && (cp <= 0xDFFF)) ||
			((cp <= 0x007F) && (num != 1)) ||
			((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
			((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
			((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
			return false;
	}

	return true;
}

void	checkKeyName(std::string keyName) throw (std::invalid_argument)
{
	for (std::size_t i = 0; i < keyName.size(); ++i)
	{
		if (!std::isdigit(keyName[i]) && !std::isalpha(keyName[i]) && keyName[i] != '-')
			throw std::invalid_argument("Error: parsing.cpp::checkKeyName: std::invalid_argument: Invalid key name");
	}
}

void	checkEscapedValues(std::string escapedValue) throw(std::invalid_argument)
{
	if (escapedValue.find(';', 0) != std::string::npos
		|| escapedValue.find('\n', 0) != std::string::npos
		|| escapedValue.find('\r', 0) != std::string::npos
		|| escapedValue.find(' ', 0) != std::string::npos
		|| escapedValue.find('\0', 0) != std::string::npos
		|| !is_valid_utf8(escapedValue.c_str()))
		throw std::invalid_argument("Error: parsing.cpp::checkEscapedValues: std::invalid_argument: Invalid character detected in string");
}

void handleTags(size_t &pos, const std::string& message, IRCMessage &result) throw (std::runtime_error)
{
	if (message[pos] == '@')
	{
		size_t end = message.find(' ', pos);
		std::string tagsStr = message.substr(pos + 1, end - pos - 1);
		pos = end + 1;

		size_t tagPos = 0;
		while (tagPos < tagsStr.size())
		{
			size_t tagEnd = tagsStr.find(';', tagPos);
			if (tagEnd == std::string::npos) tagEnd = tagsStr.size();

			Key key;
			
			// check if we have a client_prefix, is the + always at the beginning
			if (tagsStr[tagPos] == '+')
			{
				key.clientSpecificTag = true;
				tagPos++;
			}
			else
			{
				key.clientSpecificTag = false;
			}

			// check vendor
			std::string k = tagsStr.substr(tagPos, tagEnd);
			size_t vendorEnd = k.find('/');

			if (vendorEnd != std::string::npos)
				key.host = k.substr(0, vendorEnd);
		
			size_t equalPos = tagsStr.find('=', tagPos);
			if (equalPos != std::string::npos && equalPos < tagEnd)
			{
				key.keyName = tagsStr.substr(tagPos, equalPos - tagPos);
				checkKeyName(key.keyName);

				std::string escapedValue = tagsStr.substr(equalPos + 1, tagEnd - equalPos - 1);
				// do we stop everything because one tag is wrong ? not even the tag, the escaped value
				checkEscapedValues(escapedValue);
				// std::map<Key,std::string>::iterator it = result.tags.begin();
				// result.tags.insert(std::pair<Key, std::string>(key, escapedValue));
				result.tags[key] = escapedValue;
			} else {

				key.keyName = tagsStr.substr(tagPos, tagEnd - tagPos);
				checkKeyName(key.keyName);
				
				// result.tags.insert(std::pair<Key, std::string>(key, ""));
				result.tags[key] = "";
			}

			tagPos = tagEnd + 1;
		}
	}
}

void isValidNickChar(char c) throw(std::invalid_argument) {
	if (c > 0 && c != '\r' && c != '\n' && c != ' ' && c != '#' && c != '&')
		throw std::invalid_argument("Error: parsing.cpp::isValidNickChar: std::invalid_argument: Invalid nickname character in string");

	return;
}

void isValidUserChar(char c) throw(std::invalid_argument) {
	if (c > 0 && c != '\r' && c != '\n' && c != ' ')
		throw std::invalid_argument("Error: parsing.cpp::isValidUserChar: std::invalid_argument: Invalid username character in string");\
	
	return;
}

void handleSource(size_t &pos, const std::string& message, IRCMessage &result) throw (std::invalid_argument)
{
	if (message[pos] == ':')
	{
		size_t end = message.find(' ', pos);
		result.source.servername = message.substr(pos + 1, end - pos - 1);
		pos = end + 1;

		// Parse nickname, user, and host from the prefix
		size_t nickEnd = result.source.servername.find('!');
		size_t userEnd = result.source.servername.find('@');
		if (nickEnd != std::string::npos)
		{
			result.source.nickname = result.source.servername.substr(0, nickEnd);
			std::for_each(result.source.nickname.begin(), result.source.nickname.end(), isValidNickChar);
			if (userEnd != std::string::npos)
			{
				result.source.user = result.source.servername.substr(nickEnd + 1, userEnd - nickEnd - 1);
					 std::for_each(result.source.nickname.begin(), result.source.nickname.end(), isValidUserChar);
				result.source.host = result.source.servername.substr(userEnd + 1);
			}
			else
			{
				result.source.user = result.source.servername.substr(nickEnd + 1);
					 std::for_each(result.source.nickname.begin(), result.source.nickname.end(), isValidUserChar);
			}
			result.source.servername = "";
		} else if (userEnd != std::string::npos)
		{
			result.source.nickname = result.source.servername.substr(0, userEnd);
				std::for_each(result.source.nickname.begin(), result.source.nickname.end(), isValidNickChar);
			result.source.host = result.source.servername.substr(userEnd + 1);
			result.source.servername = "";
		} else
		{
			result.source.nickname = "";
		}

	}


	// std::cout << message << std::endl;
	// std::cout << pos << std::endl;
}

std::string cmds[] = {
	"NICK", "USER", "JOIN", "PART", "MODE", "TOPIC", "INVITE", "KICK",
	"PRIVMSG", "NOTICE", "PING", "PONG", "QUIT", "SQUIT", "KILL",
	"AWAY", "REHASH", "DIE", "RESTART", "SUMMON", "USERS", "WALLOPS",
	"USERHOST", "ISON", "PASS", "SERVER", "OPER", "CONNECT", "TRACE",
	"ADMIN", "INFO", "WHO", "WHOIS", "WHOWAS", "LIST", "NAMES", "LINKS",
	"TIME", "STATS", "SERVLIST", "SQUERY"
};

std::set<std::string> validCommands(cmds, cmds + 41);

 

void handleCommand(size_t &pos, const std::string& message, IRCMessage &result) throw (std::invalid_argument) {
	size_t start = pos;

	// Find the end of the command
	while (pos < message.size() && message[pos] != ' ' && message[pos] != '\r' && message[pos] != '\n') {
		++pos;
	}

	std::string command = message.substr(start, pos - start);

	std::cout << command << std::endl;

	// Validate command
	if (command.empty()) {
		throw std::invalid_argument("Error: parsing.cpp::handleCommand: std::invalid_argument: Empty command");
	}

	bool isDigit = std::isdigit(command[0]);
	if (isDigit) {
		// Check if the command is a valid three-digit number
		if (command.size() != 3 || !std::isdigit(command[1]) || !std::isdigit(command[2])) {
			throw std::invalid_argument("Error: parsing.cpp::handleCommand: std::invalid_argument: Invalid numeric command");
		}
	} else {
		// Check if the command is in the set of valid commands
		if (validCommands.find(command) == validCommands.end()) {
			throw std::invalid_argument("Error: parsing.cpp::handleCommand: std::invalid_argument: Invalid letter command");
		}
	}

	result.command = command;

}

void handleParams(size_t &pos, const std::string& message, IRCMessage &result) {
	while (pos < message.size() && message[pos] != '\r')
	{
		if (message[pos] == ':') {
			result.parameters.push_back(message.substr(pos + 1, message.size() - pos - 3)); // Remove '\r\n'
			break;
		} else {
			size_t end = message.find(' ', pos);
			if (end == std::string::npos) {
				end = message.find("\r\n", pos);
			}
			result.parameters.push_back(message.substr(pos, end - pos));
			pos = end + 1;

			while (pos < message.size() && message[pos] == ' ') {
				++pos;
			}
		}
	}
}

IRCMessage parseMessage(const std::string& message) throw(std::invalid_argument)
{
	IRCMessage result;
	size_t pos = 0;

	 // error case to reverify
	if (message.empty())
		throw std::invalid_argument("Error: parsing.cpp::parseMessage: std::invalid_argument: a message cannot be empty");
	
	while (pos < message.size() && message[pos] == ' ')
		++pos;

	// Parse tags if present
	handleTags(pos, message, result);
	while (pos < message.size() && message[pos] == ' ')
		++pos;

	// Parse source if present
	handleSource(pos, message, result);
	while (pos < message.size() && message[pos] == ' ')
		++pos;

	// Parse command
	handleCommand(pos, message, result);
	while (pos < message.size() && message[pos] == ' ') {
		++pos;
	}

	// Parse command
	handleParams(pos, message, result);

	return result;
}

void printMessage(const IRCMessage& message) {
	std::cout << "Tags: ";
	for (std::map<Key, std::string>::const_iterator it = message.tags.begin(); it != message.tags.end(); ++it) {
		std::cout << it->first.clientSpecificTag
		<< it->first.host
		<< "/"
		<< it->first.keyName
		<< "=" << it->second << "; ";
		// std::cout << it->first << "=" << it->second << "; ";
	}
	std::cout << std::endl;
	std::cout << "Source: "
	<< message.source.servername
	<< message.source.nickname
	<< "!"
	<< message.source.user
	<< "@"
	<< message.source.host
	<< std::endl;
	std::cout << "Command: " << message.command << std::endl;

	std::cout << "Parameters: ";
	for (std::vector<std::string>::const_iterator it = message.parameters.begin(); it != message.parameters.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;
}

int main(int argc, char **argv) {
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
		return (2);
	}

	// Example usage
	// std::string exampleMessage = "	:dan!d@localhost PRIVMSG #chan :Hey!\r\n";
	std::string exampleMessage = "@id=234AB	  :dan!d@localhost PRIVMSG #chan :Hey what's up!\r\n";
	std::cout << "Initial Message : " << exampleMessage << std::endl;
	// std::string exampleMessage = ":dan!d@localhost	PRIVMSG	#chan	:Hey!\r\n";
	try
	{
		IRCMessage parsedMessage = parseMessage(exampleMessage);
		printMessage(parsedMessage);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	//IRCMessage parsedMessage = parseMessage(exampleMessage);


	return 0;
}
