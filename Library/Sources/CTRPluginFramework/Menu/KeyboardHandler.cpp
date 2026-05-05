#include <Headers.hpp>
#include "CTRPluginFramework/Menu/KeyboardHandler.hpp"

namespace CTRPluginFramework {
    unsigned int limit[2];

    template<typename T>
    bool Callback(const void *input, string &error) {
        T val = *static_cast<const T*>(input);

        if (val >= limit[0] && val <= limit[1])
            return true;

        error = "The value must be between " + to_string(limit[0]) + "-" + to_string(limit[1]) + ".";
        return false;
    }

    // Explicit template instantiations (if needed)
    template bool Callback<u8>(const void *, string &);
    template bool Callback<u16>(const void *, string &);
    template bool Callback<u32>(const void *, string &);

    template<typename T>
    bool KeyboardHandler<T>::Set(const string &prompt, bool allowAbort, int maxLength, T &userInput, T defaultValue, OnEventsCallback eventsCallback) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        keyboard.CanAbort(allowAbort);
        keyboard.SetMaxLength(maxLength);
        keyboard.OnKeyboardEvent(eventsCallback);
        s8 res = keyboard.Open(userInput, defaultValue);
        return res == 0;
    }

    template<typename T>
    void KeyboardHandler<T>::ConfigureKeyboard(Keyboard &keyboard, bool allowAbort, bool useHex, int maxLength, OnCompareCallback compareCallback, u32 minValue, u32 maxValue) {
        keyboard.CanAbort(allowAbort);
        keyboard.IsHexadecimal(useHex);
        keyboard.SetMaxLength(maxLength);

        if (compareCallback) {
            keyboard.SetCompareCallback(compareCallback);
            limit[0] = minValue;
            limit[1] = maxValue;
        }
    }

    template<typename T>
    bool KeyboardHandler<T>::ExecuteKeyboard(Keyboard &keyboard, T &userInput, T defaultValue) {
        s8 result = keyboard.Open(static_cast<T&>(userInput), static_cast<T>(defaultValue));
        return result == 0;
    }

    template <typename T>
    T *GetArg(MenuEntry *entry) {
        T *argument = reinterpret_cast<T*>(entry->GetArg());

        if (!argument) {
            argument = new T();
            entry->SetArg(argument);
        }

        return argument;
    }

    template <typename T>
    T *GetArg(MenuEntry *entry, T defaultValue) {
        T *argument = reinterpret_cast<T*>(entry->GetArg());

        if (!argument) {
            argument = new T(defaultValue);
            entry->SetArg(argument);
        }

        return argument;
    }

    // Implement the specialized versions for u8
    template<>
    bool KeyboardHandler<u8>::Set(const string &prompt, bool allowAbort, bool useHex, int maxLength, u8 &userInput, u8 defaultValue, u32 minValue, u32 maxValue, bool (*compareCallback)(const void*, string&)) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        ConfigureKeyboard(keyboard, allowAbort, useHex, maxLength, compareCallback, minValue, maxValue);
        return ExecuteKeyboard(keyboard, userInput, defaultValue);
    }

    template<>
    bool KeyboardHandler<u8>::Set(const string &prompt, bool allowAbort, int maxLength, u8 &userInput, u8 defaultValue, void (*eventsCallback)(Keyboard&, KeyboardEvent&)) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        ConfigureKeyboard(keyboard, allowAbort, false, maxLength, nullptr, 0, 0);
        keyboard.OnKeyboardEvent(eventsCallback);
        return ExecuteKeyboard(keyboard, userInput, defaultValue);
    }

    // Implement the specialized versions for u16
    template<>
    bool KeyboardHandler<u16>::Set(const string &prompt, bool allowAbort, bool useHex, int maxLength, u16 &userInput, u16 defaultValue, u32 minValue, u32 maxValue, bool (*compareCallback)(const void*, string&)) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        ConfigureKeyboard(keyboard, allowAbort, useHex, maxLength, compareCallback, minValue, maxValue);
        return ExecuteKeyboard(keyboard, userInput, defaultValue);
    }

    template<>
    bool KeyboardHandler<u16>::Set(const string &prompt, bool allowAbort, int maxLength, u16 &userInput, u16 defaultValue, void (*eventsCallback)(Keyboard&, KeyboardEvent&)) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        ConfigureKeyboard(keyboard, allowAbort, false, maxLength, nullptr, 0, 0);
        keyboard.OnKeyboardEvent(eventsCallback);
        return ExecuteKeyboard(keyboard, userInput, defaultValue);
    }

    // Implement the specialized versions for u32
    template<>
    bool KeyboardHandler<u32>::Set(const string &prompt, bool allowAbort, bool useHex, int maxLength, u32 &userInput, u32 defaultValue, u32 minValue, u32 maxValue, bool (*compareCallback)(const void*, string&)) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        ConfigureKeyboard(keyboard, allowAbort, useHex, maxLength, compareCallback, minValue, maxValue);
        return ExecuteKeyboard(keyboard, userInput, defaultValue);
    }

    template<>
    bool KeyboardHandler<u32>::Set(const string &prompt, bool allowAbort, int maxLength, u32 &userInput, u32 defaultValue, void (*eventsCallback)(Keyboard&, KeyboardEvent&)) {
        Sleep(Milliseconds(100)); // Sleep for 100 milliseconds
        Keyboard keyboard(prompt);
        ConfigureKeyboard(keyboard, allowAbort, false, maxLength, nullptr, 0, 0);
        keyboard.OnKeyboardEvent(eventsCallback);
        return ExecuteKeyboard(keyboard, userInput, defaultValue);
    }

    template<>
    bool KeyboardHandler<string>::Set(const string &prompt, bool allowAbort, int maxLength, string &userInput, string defaultValue, void (*eventsCallback)(Keyboard&, KeyboardEvent&)) {
        Sleep(Milliseconds(100));
        Keyboard keyboard(prompt);
        keyboard.SetMaxLength(maxLength);
        keyboard.SetCompareCallback(nullptr);
        keyboard.OnKeyboardEvent(eventsCallback);
        int result = keyboard.Open(userInput, defaultValue);

        // Handle result if needed
        return result == 0; // Assume 0 means success
    }

    template<>
    bool KeyboardHandler<string>::Set(const string &prompt, bool allowAbort, int maxLength, string &userInput, string defaultValue) {
        return Set(prompt, allowAbort, maxLength, userInput, defaultValue, nullptr);
    }

    template class KeyboardHandler<string>;
}