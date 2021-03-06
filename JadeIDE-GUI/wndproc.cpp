#include "wndproc.h"

// Message handler for create file box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK CreateFileDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            TCHAR* newFileName = GetDialogInput(hDlg, IDC_NEWFILETEXT);

            LPCFDATA cfData = (LPCFDATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);

            if (AddProjectItem(cfData->parentPath, newFileName, cfData->createType))
            {
                size_t newFileLength = _tcslen(cfData->parentPath) + _tcslen(newFileName) + _tcslen(_T("\\")) + 1;
                TCHAR* newFilePath = new TCHAR[newFileLength];
                newFilePath[0] = _T('\0');

                //_tcscat_s(newFilePath, newFileLength - _tcslen(newFilePath), cfData->parentPath);
                //_tcscat_s(newFilePath, newFileLength - _tcslen(newFilePath), _T("\\"));
                //_tcscat_s(newFilePath, newFileLength - _tcslen(newFilePath), newFileName);

                _stprintf_s(newFilePath, newFileLength, TEXT("%s\\%s"), cfData->parentPath, newFileName);

                LPFINFO fInfo = CreateProjectFileInfo(newFileName, newFilePath, cfData->createType);

                AddItemToTree(cfData->hwndTv, fInfo, cfData->parentNode);
            }
            else
            {
                MessageBox(hDlg, L"Failed to create new file", L"Error", MB_OK);
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK RenameFileDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            TCHAR* newFileName = GetDialogInput(hDlg, IDC_NEWFILETEXT);

            LPRFDATA rfData = (LPRFDATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);

            BOOL renamed = RenameProjectFile(rfData->oldFilePath, newFileName);

            if (renamed)
            {
                TVITEM item;
                item.hItem = rfData->itemToRename;

                item.mask = TVIF_PARAM;
                TreeView_GetItem(rfData->hwndTv, &item);

                LPFINFO fInfo = (LPFINFO)item.lParam;

                fInfo->fileFullPath = ReplaceFileName(rfData->oldFilePath, newFileName);
                fInfo->fileName = newFileName;

                item.mask = TVIF_TEXT | TVIF_PARAM;
                item.pszText = newFileName;
                item.cchTextMax = _tcslen(newFileName);
                item.lParam = (LPARAM)fInfo;

                TreeView_SetItem(rfData->hwndTv, &item);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK CreateProjectDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            TCHAR* newProjectName = GetDialogInput(hDlg, IDC_NEWPROJECTFOLDER);

            if (newProjectName == NULL || _tcscmp(newProjectName, L"") == 0)
            {
                MessageBox(hDlg, L"Input is empty!", L"Error", MB_OK);
                break;
            }

            HWND hwndTv = (HWND)GetWindowLongPtr(hDlg, GWLP_USERDATA);

            LPFINFO fRootInfo = new FINFO;

            fRootInfo->fileFullPath = const_cast<TCHAR*>(newProjectName);
            fRootInfo->fileName = const_cast<TCHAR*>(newProjectName);
            fRootInfo->fType = FileType::PROOT;

            HTREEITEM hti = AddItemToTree(hwndTv, fRootInfo, TVI_ROOT);

            ListDirectoryContents(hwndTv, newProjectName, hti);

            CreateSrcPackage(hwndTv, hti, fRootInfo->fileFullPath);

            EndDialog(hDlg, (INT_PTR)hti);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)NULL;
        }
        if (LOWORD(wParam) == IDC_SELECTPROJECTFOLD)
        {

            LPCTSTR path = OpenDirectory(hDlg);

            if (path == NULL)
            {
                break;
            }

            WORD length = (WORD)SendDlgItemMessage(hDlg,
                IDC_NEWPROJECTFOLDER,
                EM_LINELENGTH,
                (WPARAM)0,
                (LPARAM)0);

            SendDlgItemMessage(hDlg,
                IDC_NEWPROJECTFOLDER,
                EM_SETSEL,
                (WPARAM)0,
                (LPARAM)-1);

            SendDlgItemMessage(hDlg,
                IDC_NEWPROJECTFOLDER,
                EM_REPLACESEL,
                (WPARAM)FALSE,
                (LPARAM)path);

            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}