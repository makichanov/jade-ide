#include "handlers.h"

LPHCSTRUCT OnCreate(HINSTANCE hInst, HWND hWnd)
{
    HWND hwndRedit = CreateRichEdit(hWnd, hInst);
    HWND hwndTv = CreateATreeView(hInst, hWnd);
    LPHCSTRUCT hcStruct = new HCSTRUCT;
    hcStruct->hwndRedit = hwndRedit;
    hcStruct->hwndTv = hwndTv;
    return hcStruct;
}

LPFINFO OnFileOpen(HWND hWnd, HWND hwndRedit, HWND hwndTv)
{
    LPFINFO fInfo = GetSelectedProjectFileInfo(hwndTv);

    if (fInfo == NULL)
    {
        return NULL;
    }

    if (fInfo->fType != FileType::PFILE)
    {
        return NULL;
    }

    TCHAR* buffer = ReadFileData(fInfo->fileFullPath);
    SetTextToREdit(hWnd, hwndRedit, buffer);
    return fInfo;
}

void OnContextMenu(HINSTANCE hInst, HWND hWnd, HWND hwndTv, LPARAM lParam)
{
    RECT rcTree;
    HTREEITEM htvItem;
    TVHITTESTINFO htInfo = { 0 };

    GetWindowRect(hwndTv, &rcTree);

    long xPos = GET_X_LPARAM(lParam) - rcTree.left;   // x position from message, in screen coordinates
    long yPos = GET_Y_LPARAM(lParam) - rcTree.top - 1;   // y position from message, in screen coordinates 

                  // get its window coordinates
    htInfo.pt.x = xPos;            // convert to client coordinates
    htInfo.pt.y = yPos;

    if (htvItem = TreeView_HitTest(hwndTv, &htInfo)) {    // hit test
        TreeView_SelectItem(hwndTv, htvItem);           // success; select the item
        TVITEM item;
        TCHAR buf[100];
        item.hItem = htvItem;
        item.mask = TVIF_PARAM;
        TreeView_GetItem(hwndTv, &item);

        HMENU hMenu;
        HMENU hmenuTrackPopup;

        hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_TREEVIEWCONTEXT));
        if (hMenu == NULL)
        {
            return;
        }

        hmenuTrackPopup = GetSubMenu(hMenu, 0);
        POINT pt;
        pt.x = xPos;
        pt.y = yPos;
        ClientToScreen(hWnd, &pt);

        LPFINFO fInfo = (LPFINFO)item.lParam;

        if (fInfo->fType == FileType::PFILE)
        {
            EnableMenuItem(hmenuTrackPopup, IDM_CONTEXTCREATEFILE, MF_GRAYED);
            EnableMenuItem(hmenuTrackPopup, IDM_CONTEXTCREATEPACKAGE, MF_GRAYED);
        }
        TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON,
            pt.x, pt.y, 0, hWnd, NULL);
    }
}

void OnContextCreateFile(HINSTANCE hInst, HWND hWnd, HWND hwndTv)
{
    HTREEITEM htvItem;
    TVITEM item;

    htvItem = TreeView_GetSelection(hwndTv);
    item.hItem = htvItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &item);

    LPFINFO fInfo = (LPFINFO)item.lParam;

    LPCFDATA cfData = CreateCFDATA(fInfo->fileFullPath, htvItem);
    cfData->hwndTv = hwndTv;
    cfData->createType = FileType::PFILE;
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CREATEFILEBOX), hWnd, CreateFileDialog, (LPARAM)cfData);
}

void OnContextCreatePackage(HINSTANCE hInst, HWND hWnd, HWND hwndTv)
{
    HTREEITEM htvItem;
    TVITEM item;

    htvItem = TreeView_GetSelection(hwndTv);
    item.hItem = htvItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &item);

    LPFINFO fInfo = (LPFINFO)item.lParam;

    LPCFDATA cfData = CreateCFDATA(fInfo->fileFullPath, htvItem);
    cfData->hwndTv = hwndTv;
    cfData->createType = FileType::PDIRECTORY;
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CREATEFILEBOX), hWnd, CreateFileDialog, (LPARAM)cfData);
}

void OnContextDelete(HINSTANCE hInst, HWND hWnd, HWND hwndTv)
{
    HTREEITEM htvItem;
    TVITEM item;

    htvItem = TreeView_GetSelection(hwndTv);
    item.hItem = htvItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &item);

    LPFINFO fInfo = (LPFINFO)item.lParam;

    switch (fInfo->fType)
    {
    case FileType::PFILE:
    {

        int answ = MessageBox(hWnd, L"Are you sure you want to delete this file?", L"Deleting file", MB_YESNO);
        if (answ == IDYES)
        {
            BOOL deleted = DeleteProjectFile(fInfo->fileFullPath);
            if (deleted)
            {
                TreeView_DeleteItem(hwndTv, htvItem);
                MessageBox(hWnd, L"File was successfully deleted", L"Successfully deleted", MB_OK);
            }
        }
    }
    break;
    case FileType::PDIRECTORY:
    {
        if (PathIsDirectoryEmpty(fInfo->fileFullPath))
        {
            if (RemoveDirectory(fInfo->fileFullPath))
            {
                TreeView_DeleteItem(hwndTv, htvItem);
                MessageBox(hWnd, L"Package was successfully deleted", L"Successfully deleted", MB_OK);
            }
        }
        else
        {
            int answ = MessageBox(hWnd, L"Are you sure you want to delete this package?", L"Deleting package", MB_YESNO);
            if (answ == IDYES)
            {
                if (DeleteNonEmptyPackage(fInfo->fileFullPath))
                {
                    TreeView_DeleteItem(hwndTv, htvItem);
                    MessageBox(hWnd, L"Package was successfully deleted", L"Successfully deleted", MB_OK);
                }
            }
        }
    }
    break;
    default:
        break;
    }
}

void OnContextRename(HINSTANCE hInst, HWND hWnd, HWND hwndTv)
{
    HTREEITEM htvItem;
    TVITEM item;

    htvItem = TreeView_GetSelection(hwndTv);
    item.hItem = htvItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &item);

    LPFINFO fInfo = (LPFINFO)item.lParam;

    LPRFDATA rfData = CreateRFDATA(fInfo->fileFullPath, hwndTv, htvItem);
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENAMEFILEBOX), hWnd, RenameFileDialog, (LPARAM)rfData);
}

HTREEITEM OnContextMarkAsMain(HINSTANCE hInst, HWND hWnd, HWND hwndTv, HTREEITEM htiOld)
{
    HTREEITEM hti = TreeView_GetSelection(hwndTv);
    TVITEM tv;
    tv.hItem = hti;
    tv.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &tv);
    if (CreateMainPropertyFile((LPFINFO)tv.lParam))
    {
        SetMainIcon(hwndTv, hti, htiOld);
    }
    return hti;
}

HTREEITEM OnContextMarkAsSrc(HINSTANCE hInst, HWND hWnd, HWND hwndTv, HTREEITEM htiOld)
{
    HTREEITEM hti = TreeView_GetSelection(hwndTv);
    TVITEM tv;
    tv.hItem = hti;
    tv.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &tv);
    if (CreateSrcPropertyFile((LPFINFO)tv.lParam))
    {
        MarkPackageAsSource(hwndTv, hti, htiOld);
    }
    return hti;
}

void OnSaveFile(LPFINFO lpCurrentFile, HWND hWnd, HWND hwndRedit)
{

    TCHAR* buffer = GetREditText(hWnd, hwndRedit);
    if (!WriteToFile(lpCurrentFile->fileFullPath, buffer))
    {
        MessageBox(hWnd, L"Failed to save file", L"Error", MB_OK);
    }
}

HTREEITEM OnCreateProject(HINSTANCE hInst, HWND hWnd, HWND hwndTv)
{
    HTREEITEM root = (HTREEITEM)DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CREATEPROJECTBOX), hWnd, CreateProjectDialog, (LPARAM)hwndTv);
    if (root == (HTREEITEM)2)
    {
        return NULL;
    }
    return root;
}

HTREEITEM OnOpenProject(HWND hWnd, HWND hwndTv, HTREEITEM treeRoot)
{
    LPCTSTR path = OpenDirectory(hWnd);
    if (path == NULL)
    {
        return treeRoot;
    }
    if (treeRoot != NULL)
    {
        TreeView_DeleteAllItems(hwndTv);
    }
    LPFINFO pInfo = CreateProjectFileInfo(const_cast<LPTSTR>(path), const_cast<LPTSTR>(path), FileType::PROOT);
    treeRoot = AddItemToTree(hwndTv, pInfo, TVI_ROOT);
    ListDirectoryContents(hwndTv, path, treeRoot);

    
    
    return treeRoot;
}

HTREEITEM OnCloseProject(HWND hwndTv, HTREEITEM treeRoot)
{
    if (treeRoot != NULL)
    {
        TreeView_DeleteAllItems(hwndTv);
        return NULL;
    }
    else
    {
        return treeRoot;
    }
}

void OnSize(HWND hWnd, HWND hwndTv, HWND hwndRedit)
{
    RECT rect;
    GetClientRect(hWnd, &rect);

    //treeview
    SetWindowPos(hwndTv,
        NULL,
        0,
        0,
        rect.right * 0.25,
        rect.bottom,
        NULL);

    //richedit
    SetWindowPos(hwndRedit,
        NULL,
        rect.right * 0.25,
        rect.top,
        rect.right,
        rect.bottom,
        0);
}

void OnExit(HWND hWnd)
{
    DestroyWindow(hWnd);
}

void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...
    EndPaint(hWnd, &ps);
}

void OnSelectJDK(HWND hWnd, HWND hwndTv, HTREEITEM root)
{
    if (root == NULL)
    {
        MessageBox(hWnd, L"Need to open project first", L"Error", MB_OK);
        return;
    }
    LPCTSTR pathToJDK = OpenDirectory(hWnd);

    if (ValidateJDK(const_cast<TCHAR*>(pathToJDK)))
    {
        TVITEM tvi;
        tvi.hItem = root;
        tvi.mask = TVIF_PARAM;
        TreeView_GetItem(hwndTv, &tvi);
        LPFINFO fInfo = (LPFINFO)tvi.lParam;

        TCHAR* pathToJdkProp = AppendFileName(fInfo->fileFullPath, const_cast<TCHAR*>(JDKPATH));

        WriteToFile(pathToJdkProp, const_cast<TCHAR*>(pathToJDK));
    }
    else
    {
        MessageBox(hWnd, L"Failed to validate JDK", L"Error", MB_OK);
    }
}

BOOL OnBuildRun(HWND hwndTv, HTREEITEM root, HTREEITEM src)
{
    if (root == NULL)
    {
        return FALSE;
    }
    TVITEM tvi;
    tvi.hItem = root;
    tvi.mask = TVIF_PARAM;
    TreeView_GetItem(hwndTv, &tvi);
    LPFINFO fInfo = (LPFINFO)tvi.lParam;

    TCHAR* pathToJdkProp = AppendFileName(fInfo->fileFullPath, const_cast<TCHAR*>(JDKPATH));

    if (!PathFileExists(pathToJdkProp))
    {
        return FALSE;
    }

    BuildProject(hwndTv, src, root);

}