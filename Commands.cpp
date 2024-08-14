void handleKick(Channel& channel, const std::string& targetNickname) {
    if (channel.users.find(targetNickname) != channel.users.end()) {
        channel.users.erase(targetNickname);
        std::cout << targetNickname << " has been kicked from " << channel.name << "\n";
    } else {
        std::cout << "User " << targetNickname << " not found in channel " << channel.name << "\n";
    }
}

void handleInvite(Channel& channel, User& inviter, const std::string& inviteeNickname) {
    if (channel.inviteOnly) {
        channel.invitedUsers.insert(inviteeNickname);
        std::cout << inviteeNickname << " has been invited to " << channel.name << " by " << inviter.nickname << "\n";
    } else {
        std::cout << "Channel is not invite-only. Invite unnecessary.\n";
    }
}

void handleTopic(Channel& channel, const std::string& newTopic, bool isOperator) {
    if (channel.topicRestricted && !isOperator) {
        std::cout << "You do not have permission to change the topic.\n";
    } else {
        channel.topic = newTopic;
        std::cout << "The topic for " << channel.name << " has been changed to: " << channel.topic << "\n";
    }
}

void displayTopic(const Channel& channel) {
    std::cout << "The topic for " << channel.name << " is: " << channel.topic << "\n";
}

void handleMode(Channel& channel, char mode, const std::string& param, bool add) {
    switch (mode) {
        case 'i': // Invite-only mode
            channel.inviteOnly = add;
            std::cout << "Invite-only mode " << (add ? "enabled" : "disabled") << " for " << channel.name << "\n";
            break;
        case 't': // Topic restricted mode
            channel.topicRestricted = add;
            std::cout << "Topic-restricted mode " << (add ? "enabled" : "disabled") << " for " << channel.name << "\n";
            break;
        case 'k': // Channel key (password)
            if (add) {
                channel.key = param;
                std::cout << "Key set for " << channel.name << "\n";
            } else {
                channel.key.clear();
                std::cout << "Key removed for " << channel.name << "\n";
            }
            break;
        case 'o': // Operator privilege
            if (add) {
                channel.operators.insert(param);
                std::cout << param << " is now an operator in " << channel.name << "\n";
            } else {
                channel.operators.erase(param);
                std::cout << param << " is no longer an operator in " << channel.name << "\n";
            }
            break;
        case 'l': // User limit
            if (add) {
                channel.userLimit = std::stoi(param);
                std::cout << "User limit set to " << channel.userLimit << " for " << channel.name << "\n";
            } else {
                channel.userLimit = 0;
                std::cout << "User limit removed for " << channel.name << "\n";
            }
            break;
        default:
            std::cout << "Unknown mode: " << mode << "\n";
            break;
    }
}

int main() {
    // Sample channel and user
    Channel channel("example");
    User user("john");

    // Example usage
    handleMode(channel, 'i', "", true);  // Enable invite-only mode
    handleInvite(channel, user, "doe");  // Invite "doe" to the channel
    handleKick(channel, "john");         // Kick "john" from the channel
    handleTopic(channel, "New Topic", true); // Set new topic as an operator

    return 0;
}
