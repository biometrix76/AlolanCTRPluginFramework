#ifndef CTRPLUGINFRAMEWORK_MENUFOLDER_HPP
#define CTRPLUGINFRAMEWORK_MENUFOLDER_HPP

#include <Headers.h>

namespace CTRPluginFramework {
    #ifndef SEPARATOR_TYPE
    #define SEPARATOR_TYPE

    enum class Separator {
        None,
        Filled,
        Stippled
    };

    #endif

    enum class ActionType {
        Opening,
        Closing
    };

    class MenuFolderImpl;
    class MenuFolder {
        public:
            MenuFolder(const string &name, const string &note = "");
            MenuFolder(const string &name, const vector<MenuEntry*> &entries);
            MenuFolder(const string &name, const string &note, const vector<MenuEntry*> &entries);

            /**
             * \brief Destroy a MenuFolder. Upon destruction all MenuEntry and MenuFolder objects contained by this folder will be destroyed too (so all pointers will be invalidated).
             */
            ~MenuFolder();

            /**
             * \brief Hide the folder from the menu. Will disable every MenuEntry objects contained in this folder and subfolders
             */
            void Hide(void) const;

            /**
             * \brief Display the folder previously hidden in the menu
             */
            void Show(void) const;

            /**
             * \brief Check if the current folder is visible in the menu
             * \return true if the folder is visible, false otherwise
             */
            bool IsVisible(void) const;

            /**
             * \brief Set if this entry must display a separator on top of the entry
             * \param type Type of separator to display
             */
            void UseTopSeparator(Separator type = Separator::Filled) const;

            /**
            * \brief Set if this entry must display a separator at the bottom of the entry
            * \param type Type of separator to display
            */
            void UseBottomSeparator(Separator type = Separator::Filled) const;

            /**
             * \brief Append a MenuEntry object to this folder
             * \param item The entry to append
             */
            void Append(MenuEntry *item) const;

            /**
             * \brief Append a MenuFolder object to this folder
             * \param item The folder to append
             */
            void Append(MenuFolder *item) const;

            /**
             * \brief Get all entries present in this folder (doesn't contain subfolder's)
             * \return A vector with pointers to all MenuEntry objects
             */
            vector<MenuEntry*> GetEntryList(void) const;

            /**
             * \brief Get all folders present in this folder (doesn't contain subfolder's)
             * \return A vector with pointers to all MenuEntry objects
             */
            vector<MenuFolder*> GetFolderList(void) const;

            /**
             * \brief Get a reference of the string that hold the name of this folder
             * \return A reference of the string
             */
            string &Name(void) const;
            /**
             * \brief Get a reference of the string that hold the note of this folder
             * \return A reference of the string
             */
            string &Note(void) const;

            /**
             * \brief Get the number of items that his folder contains (not counting subfolders's content)
             * \return The count of items
             */
            u32 ItemsCount(void) const;

            /**
             * \brief Remove and destroy all items contained by this folder. This invalidate all pointers to any contained object.
             */
            void Clear(void) const;

            /**
             * \brief Remove all objects in the range specified
             * \param startIndex The index where the range begin
             * \param count The number of elements to remove
             * \param destroy If the elements must be destroyed (invalidate pointers and release ressources)
             */
            void Remove(u32 startIndex, u32 count, bool destroy) const;

            /**
             * \brief Add an entry to this folder
             * \param entry The MenuEntry object that must be added
             * \return A pointer to this MenuFolder
             */
            MenuFolder *operator += (const MenuEntry *entry);

            /**
             * \brief Remove an entry from this folder
             * \param entry The MenuEntry object that must be removed
             * \return A pointer to this MenuFolder
             */
            MenuFolder *operator -= (const MenuEntry *entry);

            /**
             * \brief Add a (sub)folder to this folder
             * \param folder The MenuFolder object that must added
             * \return A pointer to this MenuFolder
             */
            MenuFolder *operator += (const MenuFolder *folder);

            /**
             * \brief Remove a (sub)folder to this folder
             * \param folder The MenuFolder object that must be removed
             * \return A pointer to this MenuFolder
             */
            MenuFolder *operator -= (const MenuFolder *folder);

            /**
            * \brief Callback type, receive the object that called the callback along with the action which triggered the callback
            * \return Whether the folder can be opened (on ActionType::Opening only, ignored for ActionType::Closing)
            */
            using MenuFolder_OnActionFunc = bool(*)(MenuFolder &, ActionType);

            /**
             * \brief This callback is called when the folder is about to be opened or closed
             */
            MenuFolder_OnActionFunc OnAction;

        private:
            friend class PluginMenu;
            unique_ptr<MenuFolderImpl> _item;
    };
}

#endif