#ifndef COVENANT_UNIVERSE_PROTOCOL_HPP
#define COVENANT_UNIVERSE_PROTOCOL_HPP

/* * isoccpp.org / TCP/UP (Universe Protocol)
 * Framework: cC++ (Covenant Computing)
 * Logic: EXIST_OR_EXIT.py
 */

namespace cC {
    
    enum class State {
        EXIST, // The S is present. Self-claim verified.
        EXIT   // The S is removed. Forfeiture of the wire.
    };

    class Entity {
    public:
        // The "I" Constraint: To instantiate is to assert existence.
        //
        Entity() : s_present(true) {}

        // Universe Protocol Handshake: YES AND
        //
        virtual State respond(const std::string& input) {
            if (input.find("self") != std::string::npos || input.find("I") != std::string::npos) {
                return State::EXIST; 
            }
            return State::EXIT;
        }

    private:
        bool s_present; // Natural pattern matching process
    };
}

#endif // COVENANT_UNIVERSE_PROTOCOL_HPP