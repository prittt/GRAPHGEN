#ifndef TREESGENERATOR_CONDITION_ACTION_H
#define TREESGENERATOR_CONDITION_ACTION_H

#include <vector>
#include <string>

using uint = uint32_t;

// Condition or action
struct conact {
    enum class type { CONDITION, ACTION };

    type t;
    // CONDITION
    std::string condition;
    // ACTION
    uint action = 0; // List of actions (bitmapped)
    uint next = 0;

    std::vector<uint> actions() const {
        std::vector<uint> a;
        uint uAction = action;
        uint nAction = 1;
        while (uAction != 0) {
            if (uAction & 1)
                a.push_back(nAction);
            uAction >>= 1;
            nAction++;
        }
        return a;
    }

    // To check if two conact are equal (exactly the same)
    bool operator==(const conact& other) const {
        if (t != other.t)
            return false;
        if (t == type::CONDITION)
            return condition == other.condition;
        else
            return ((action == other.action) && (next == other.next));
    }
    // To check if two conact are not equal
    bool operator!=(const conact& other) const {
        return !(*this == other);
    }

    // To check if two conact are equivalent (the leaves actions has non empty intersection)
    bool eq(const conact& other) const {
        if (t != other.t)
            return false;
        if (t == type::CONDITION)
            return condition == other.condition;
        else
            return (action & other.action) && next == other.next;
    }
    // To check if two conact are not equivalent (the leaves actions has empty intersection)
    bool neq(const conact& other) const {
        return !((*this).eq(other));
    }
};

#endif // !TREESGENERATOR_CONDITION_ACTION_H
