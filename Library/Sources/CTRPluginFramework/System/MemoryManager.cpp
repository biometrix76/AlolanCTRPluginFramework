#include <Headers.hpp>

namespace CTRPluginFramework {
    MemoryManager::MemoryManager(u32 address)
        : address_(address), originalValue_(0), isValueSet_(false) {
        // Attempt to read and store the original value
        if (Process::Read32(address_, originalValue_))
            isValueSet_ = true;
    }

    MemoryManager::~MemoryManager() {
        // Restore the original value when the object is destroyed
        if (isValueSet_)
            Process::Write32(address_, originalValue_);
    }

    bool MemoryManager::Write(u32 newValue) {
        if (!isValueSet_) return false;
        return Process::Write32(address_, newValue);
    }

    bool MemoryManager::HasOriginalValue() const {
        return isValueSet_;
    }

    u32 MemoryManager::GetOriginalValue() const {
        return originalValue_;
    }

    bool MemoryManager::RestoreOriginalValue() {
        if (isValueSet_)
            return Process::Write32(address_, originalValue_);

        return false;
    }

    bool MemoryManager::SaveCurrentAsOriginal() {
        u32 currentValue;

        if (Process::Read32(address_, currentValue)) {
            originalValue_ = currentValue;
            isValueSet_ = true;
            return true;
        }

        return false;
    }
}