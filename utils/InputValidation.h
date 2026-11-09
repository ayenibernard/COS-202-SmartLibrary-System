#pragma once
#include <cctype>
#include <string>

class InputValidation {
public:
    static bool validId(const std::string& value) {
        if (value.empty() || value.size() > 32) return false;
        for (unsigned char ch : value)
            if (!std::isalnum(ch) && ch != '_' && ch != '-') return false;
        return true;
    }
    static bool meaningfulText(const std::string& value, std::size_t maxLength = 160) {
        if (value.empty() || value.size() > maxLength) return false;
        bool hasNonSpace = false;
        for (unsigned char ch : value) {
            if (ch < 32 || ch == 127) return false;
            if (!std::isspace(ch)) hasNonSpace = true;
        }
        return hasNonSpace;
    }
    static std::string item(const std::string& id, const std::string& title,
                            const std::string& author, int year) {
        if (!validId(id)) return "Item ID must contain 1-32 letters, digits, underscores or hyphens.";
        if (!meaningfulText(title)) return "Title must contain 1-160 characters of meaningful text.";
        if (!meaningfulText(author)) return "Author must contain 1-160 characters of meaningful text.";
        if (year < 1 || year > 9999) return "Publication year must be between 1 and 9999.";
        return {};
    }
    static std::string user(const std::string& id, const std::string& name) {
        if (!validId(id)) return "User ID must contain 1-32 letters, digits, underscores or hyphens.";
        if (!meaningfulText(name)) return "User name must contain 1-160 characters of meaningful text.";
        return {};
    }
};
