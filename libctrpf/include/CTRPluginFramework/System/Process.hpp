#ifndef CTRPLUGINFRAMEWORK_PROCESS_HPP
#define CTRPLUGINFRAMEWORK_PROCESS_HPP

#include <Headers.h>

namespace CTRPluginFramework {
    enum class StringFormat {
        Utf8,
        Utf16,
        Utf32
    };

    class MenuEntry;
    class Process {
        using FuncPointer = void(*)(MenuEntry*);
        public:
            /**
             * \brief Get current process handle (already open, don't close it)
             * \return Return current process handle (already open, don't close it)
             */
            static Handle GetHandle(void);

            /**
             * \brief Get current process ID
             * \return Return current process ID
             */
            static u32 GetProcessID(void);

            /**
             * \brief Get the title ID of the current process
             * \return TitleID of the current process
             */
            static u64 GetTitleID(void);

            /**
             * \brief Get the title ID of the current process as string
             * \param output String to append the TitleID to output isn't clear
             */
            static void GetTitleID(string &output);

            /**
             * \brief Get the name of the current process
             * \param output string to output the process name
             */
            static void GetName(string &output);

            /**
             * \brief Get the version of the current process
             * \return The version of the current processs
             */
            static u16 GetVersion(void);

            /**
             * \brief Get the total size of the .text section
             * \return The total size of the .text section
             */
            static u32 GetTextSize(void);
            static u32 GetRoDataSize(void);
            static u32 GetRwDataSize(void);

            /**
             * \brief Check if the process is currently paused by the framework (Like when a Keyboard, MessageBox or the menu is open)
             * \return True if the process is paused
             */
            static bool IsPaused(void);

            /**
             * \brief Pause the process at the next frame
             */
            static void Pause();

            using OnPauseResumeCallback = void(*)(bool goingToPause);

            /**
            * \brief Called when the game is paused by CTRPF
            */
            static OnPauseResumeCallback OnPauseResume;

            /**
             * \brief Resume the process
             * \param frames If a number is specified the process will play x framesm before being paused again. Please be aware that giving a frame count will not release the process
             */
            static void Play(const u32 frames = 0);

            /**
             * @brief Waits until the application exits. Useful if PluginMenu::Run is not used in main() to block execution until the app exits.
             */
            static void WaitForExit(void);

            /**
            * \brief Safely patch the current process (MemPerm check and Invalidate cache)
            * \param addr The address to start the patch
            * \param patch The patch values
            * \param length The length of the patch values in bytes
            * \param original If defined, the original values will be copied to this buffer (size must be enough)
            * \return If the patch was successful or not
            */
            static bool Patch(u32 addr, void *patch, u32 length, void *original = nullptr);

            /**
            * \brief Safely patch the current process (MemPerm check and Invalidate cache)
            * \param addr The address to start the patch
            * \param patch The patch value
            * \param original If defined, the original values will be copied to this buffer (size must be enough)
            * \return If the patch was successful or not
            */
            static bool Patch(u32 addr, u32 patch, void *original = nullptr);

            /**
            * \brief Protect the memory by settings Read & Write perm
            * \param addr Address from start protecting
            * \param size Size to protect
            * \param perm The permission to apply (RWX by default)
            * \return If the protection was successful or not
            */
            static bool ProtectMemory(u32 addr, u32 size, int perm = (MEMPERM_READ | MEMPERM_WRITE |MEMPERM_EXECUTE));

            /**
             * \brief Protect the entire region where addr belongs to
             * \param addr An address
             * \param perm The permissions that must be applied to the region
             * \return  If the protection was successful or not
             */
            static bool ProtectRegion(u32 addr, int perm = (MEMPERM_READ | MEMPERM_WRITE |MEMPERM_EXECUTE));

            /**
             * \brief Protect all regions within the provided range
             * \param startAddress The address from the range starts
             * \param endAddress The address from the range ends
             * \param perm The permissions that muse be applied to the regions
             */
            static void ProtectRegionInRange(u32 startAddress, u32 endAddress, int perm = (MEMPERM_READ | MEMPERM_WRITE |MEMPERM_EXECUTE));

            /**
             * \brief Safely copy the current process memory (MemPerm check) \n Like a memcpy but safe
             * \param dst The destination to copy to
             * \param src The source to copy from
             * \param size The size to copy in bytes
             * \return
             */
            static bool CopyMemory(void *dst, const void *src, u32 size);

            /**
             * \brief Check if the address is available and have the specified permissions. If the address exists but don't have the specified permissions, it tries to change the permissions
             * \param address The address to check
             * \param perm The permissions that the address must have
             * \return If the address is valid and have the specified permissions or not
             */
            static bool CheckAddress(u32 address, u32 perm = MEMPERM_READ | MEMPERM_WRITE);

            /**
            * \brief Check if the region which the address is from is available and have the specified permissions. If the region exists but don't have the specified permissions, it tries to change the permissions
            * \param address The address to check
            * \param size A reference to get the region's size
            * \param perm The permissions that the region must have
            * \return If the region is valid and have the specified permissions or not
            */
            static bool CheckRegion(u32 address, u32 &size, u32 perm = MEMPERM_READ | MEMPERM_WRITE);

            /**
            * \brief Gets the address of the best fitting memory region bigger or equal than the specified size and past the starting address. Doesn't guarantee that the process will try to use the same region later.
            * \param size Minimum size of the memory region
            * \param startAddress Starting address to search from
            * \return Adress of the best fitting memory region bigger than size and past startAddress
            */
            static u32 GetFreeMemoryRegion(u32 size, u32 startAddress = 0x100000);

            /**
             * \brief Safely write 64 bits
             * \param address Address to write to
             * \param value The value to write
             * \return True if success, false otherwise
             */
            static bool Write64(u32 address, u64 value);

            /**
            * \brief Safely write 32 bits
            * \param address Address to write to
            * \param value The value to write
            * \return True if success, false otherwise
            */
            static bool Write32(u32 address, u32 value);

            /**
            * \brief Safely write 32 bits
            * \param pointer The pointer to read
            * \param offset The offset to jump to
            * \param value The variable to put the value in
            */
            static void Write32(u32 pointer, u32 offset, u32 value);

            /**
            * \brief Safely write 32 bits
            * \param pointer The pointer to read
            * \param offset The offset to jump to
            * \param value The variable to put the value in
            * \param length Loop and increment up until length is reached
            */
            static void Write32(u32 pointer, u32 offset, u32 value, int length);

            /**
            * \brief Safely write 32 bits
            * \param address The address to write to
            * \param value The variable to put the values in
            */
            static bool Write32(u32 address, vector<u32> value);

            /**
            * \brief Safely write 32 bits
            * \param address The address to write to
            * \param value The variable to put the values in
            * \param length Loop and increment up until length is reached
            */
            static void Write32(u32 address, vector<u32> value, int length);

            /**
            * \brief Safely write 32 bits
            * \param address The addresses to write to
            * \param value The variable to put the values in
            */
            static void Write32(vector<u32> address, vector<u32> value);

            /**
            * \brief Safely write 32 bits
            * \param address The address to write to
            * \param value The variable to put the value in
            * \param original The variable to store the initial value of the address before changes
            * \param entry Used to check for activation of entry, will revert changes upon de-activation
            * \param saved If entry is actiavted or not
            * \return True if successful, false otherwise
            */
            static bool Write32(u32 address, u32 value, u32 &original, MenuEntry *entry, bool &saved);

            /**
            * \brief Safely write 32 bits
            * \param address The address to write to
            * \param value The variable to put the values in
            * \param original The variable to store the initial value of the address before changes
            * \param entry Used to check for activation of entry, will revert changes upon de-activation
            * \param saved If saved or not
            * \return True if successful, false otherwise
            */
            static bool Write32(u32 address, vector<u32> value, vector<u32> &original, MenuEntry *entry, bool &saved);

            /**
            * \brief Safely write 32 bits
            * \param address The addresses to write to
            * \param value The variable to put the values in
            * \param original The variable to store the initial value of the address before changes
            * \param entry Used to check for activation of entry, will revert changes upon de-activation
            * \param saved If saved or not
            * \return True if successful, false otherwise
            */
            static bool Write32(vector<u32> address, vector<u32> value, vector<u32> &original, MenuEntry *entry, bool &saved);

            /**
            * \brief Safely write 16 bits
            * \param address Address to write to
            * \param value The value to write
            * \return True if success, false otherwise
            */
            static bool Write16(u32 address, u16 value);

            /**
            * \brief Safely write 16 bits
            * \param pointer The pointer to read
            * \param offset The offset to jump to
            * \param value The variable to put the value in
            */
            static void Write16(u32 pointer, u32 offset, u16 value);

            /**
            * \brief Safely write 16 bits
            * \param pointer The pointer to read
            * \param offset The offset to jump to
            * \param value The variable to put the value in
            * \param length Loop and increment up until length is reached
            */
            static void Write16(u32 pointer, u32 offset, u16 value, int length);

            /**
            * \brief Safely write 8 bits
            * \param address Address to write to
            * \param value The value to write
            * \return True if success, false otherwise
            */
            static bool Write8(u32 address, u8 value);

            /**
            * \brief Safely write 8 bits
            * \param pointer The pointer to read
            * \param offset The offset to jump to
            * \param value The variable to put the value in
            */
            static void Write8(u32 pointer, u32 offset, u8 value);

            /**
            * \brief Safely write 8 bits
            * \param pointer The pointer to read
            * \param offset The offset to jump to
            * \param value The variable to put the value in
            * \param length Loop and increment up until length is reached
            */
            static void Write8(u32 pointer, u32 offset, u8 value, int length);

            /**
            * \brief Safely write a float value
            * \param address Address to write to
            * \param value The value to write
            * \return True if success, false otherwise
            */
            static bool WriteFloat(u32 address, float value);

            /**
            * \brief Safely write a double value
            * \param address Address to write to
            * \param value The value to write
            * \return True if success, false otherwise
            */
            static bool WriteDouble(u32 address, double value);

            /**
             * \brief Safely read 64 bits
             * \param address The address to read
             * \param value The variable to put the value in
             * \return True if success, false otherwise
             */
            static bool Read64(u32 address, u64 &value);

            /**
            * \brief Safely read 32 bits
            * \param address The address to read
            * \param value The variable to put the value in
            * \return True if success, false otherwise
            */
            static bool Read32(u32 address, u32 &value);

            /**
            * \brief Safely read 32 bits
            * \param address The address to read from
            * \return The value of the address
            */
            static u32 Read32(u32 address);

            /**
            * \brief Safely read 32 bits
            * \param address The addresses to read from
            * \param length Loop and increment up until length is reached
            * \return The values of the addresses
            */
            static vector<u32> Read32(u32 address, int length);

            /**
            * \brief Safely read 16 bits
            * \param address The address to read
            * \param value The variable to put the value in
            * \return True if success, false otherwise
            */
            static bool Read16(u32 address, u16 &value);

            /**
            * \brief Safely read 16 bits
            * \param address The address to read from
            * \return The value of the address
            */
            static u16 Read16(u32 address);
            static u16 Read16(u32 pointer, u32 offset);

            /**
            * \brief Safely read 8 bits
            * \param address The address to read
            * \param value The variable to put the value in
            * \return True if success, false otherwise
            */
            static bool Read8(u32 address, u8 &value);

            /**
            * \brief Safely read 8 bits
            * \param address The address to read from
            * \return The value of the address
            */
            static u8 Read8(u32 address);

            /**
            * \brief Safely read 8 bits
            * \param address The addresses to read from
            * \param check An extra check, just to be safe
            * \return True if read is successful, false otherwise
            */
            bool Read8(u32 address, u8 &value, bool check);

            /**
            * \brief Safely read a float value
            * \param address The address to read
            * \param value The variable to put the value in
            * \return True if success, false otherwise
            */
            static bool ReadFloat(u32 address, float &value);

            /**
            * \brief Safely read a double value
            * \param address The address to read
            * \param value The variable to put the value in
            * \return True if success, false otherwise
            */
            static bool ReadDouble(u32 address, double &value);

            /**
             * \brief Read a string from the desired address and convert it to utf8
             * \param address The address to read the string from
             * \param output The string object with the utf8 string
             * \param size The size in bytes to read from the address
             * \param format The input format to do the conversion
             * \return True if the read is successful, false otherwise
             */
            static bool ReadString(u32 address, string &output, u32 size, StringFormat format);

            /**
             * \brief Write a utf8 string to an address and do the conversion if needed
             * \param address The address to write the string to
             * \param input The string to write
             * \param outFmt The desired output format
             * \return True if the write is successful, false otherwise
             */
            static bool WriteString(u32 address, const string &input, StringFormat outFmt = StringFormat::Utf8);

            /**
            * \brief Write a utf8 string to an address and do the conversion if specified
            * \param address The address to write the string to
            * \param input The string to write
            * \param size The number of bytes to write, last is null terminator
            * \param outFmt The desired output format
            * \return True if the write is successful, false otherwise
            */
            static bool WriteString(u32 address, const string &input, u32 size, StringFormat outFmt = StringFormat::Utf8);

            /**
            * \brief Causes the app to exit and jump to the home menu. Does not return.
            */
            static void ReturnToHomeMenu(void) NORETURN;

            enum ExceptionCallbackState {
                EXCB_LOOP,
                EXCB_DEFAULT_HANDLER,
                EXCB_REBOOT,
                EXCB_RETURN_HOME
            };

            using ExceptionCallback = ExceptionCallbackState(*)(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
            static ExceptionCallback exceptionCallback;
            static bool ThrowOldExceptionOnCallbackException;
    };
}

#endif