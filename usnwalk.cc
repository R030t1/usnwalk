#define _UNICODE
#include <stdlib.h>
#include <stdio.h>

#include <Windows.h>
#include <winioctl.h>

#include <string>
#include <locale>
#include <codecvt>
#include <iostream>
using namespace std;


int main(int argc, char *argv[]) {
    //std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    //std::wcout.imbue(utf8);

    DWORD rc = 0, n = 0;
    // TODO: Can't seem to open as overlapped.
    HANDLE hVol = CreateFileW(
        L"\\\\?\\C:",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, NULL, NULL
    );
    if (hVol == INVALID_HANDLE_VALUE) {
        cout << "error: CreateFileW: " << 
            GetLastError() << endl;
        exit(1);
    }

    /*HANDLE hVolIocp = CreateIoCompletionPort(
        hVol, NULL, 0, 0
    );
    if (!hVolIocp) {
        cout << "error: CreateIoCompletionPort: " <<
            GetLastError() << endl;
        exit(1);
    }*/

    USN_JOURNAL_DATA ujd = { 0 };
    n = 0;
    if (!(rc = DeviceIoControl(
        hVol, FSCTL_QUERY_USN_JOURNAL,
        NULL, 0,
        &ujd, sizeof USN_JOURNAL_DATA,
        &n, NULL
    ))) {
        cout << "error: DeviceIoControl(FSCTL_QUERY_USN_JOURNAL): " <<
            GetLastError() << endl;
        exit(1);
    }

    cout << "Journal ID: " << ujd.UsnJournalID << endl;
    cout << "First USN:  " << ujd.FirstUsn << endl;

    return EXIT_SUCCESS;

    // If you're on > Windows 8.1 must initialize the version headers. This
    // breaks backwards compat? Can't get the _V0 version working.
    MFT_ENUM_DATA_V1 med = {
        .StartFileReferenceNumber = 0,
        .LowUsn = 0,
        .HighUsn = ujd.NextUsn,
        .MinMajorVersion = 0,
        .MaxMajorVersion = 2, // Set this to 3 to get v3 records back, useful
        // for ReFS only?
    };
    uint8_t buffer[8192];
    memset(buffer, 0, sizeof buffer);
    while (true) {
        n = 0;
        // DeviceIoControl returns 0 on success, contrary to general documentation.
        if (!(rc = DeviceIoControl(
            hVol, FSCTL_ENUM_USN_DATA,
            &med, sizeof med,
            buffer, sizeof buffer,
            &n, NULL
        ))) {
            int en = GetLastError();
            if (en == ERROR_HANDLE_EOF)
                break;
            cout << "error: DeviceIoControl(FSCTL_ENUM_USN_DATA): " <<
                GetLastError() << " " << n << endl;
            exit(1);
        }

        USN u = ((USN *)buffer)[0]; 
        USN_RECORD_V2 *ur = (USN_RECORD_V2 *)(buffer + sizeof USN);

        printf("%llu\n", (uint64_t)u);
        //wprintf(L"%u.%u\n", ur->MajorVersion, ur->MinorVersion);
        wcout << wstring(ur->FileName, ur->FileNameLength / 2) << endl;

        med.StartFileReferenceNumber = u;
    }

    /*
    uint8_t recs[4096];
    memset(recs, 0, sizeof recs);
    READ_USN_JOURNAL_DATA rujd = {
        .StartUsn = 0,
        .ReasonMask = 0xFFFFFFFF,
        .ReturnOnlyOnClose = FALSE,
        .Timeout = 0,
        .BytesToWaitFor = 0,
        .UsnJournalID = ujd.UsnJournalID,
    };
    while (true) {
        n = 0;
        if (rc = DeviceIoControl(
            hVol, FSCTL_READ_USN_JOURNAL,
            &rujd, sizeof rujd,
            &recs, sizeof recs,
            &n, NULL
        )) {
            cout << "error: DeviceIoControl(FSCTL_READ_USN_JOURNAL): " <<
                GetLastError() << endl;
            exit(1);
        }

        USN u = ((USN *)recs)[0];
        printf("%llu\n", u);
        cout << GetLastError() << endl;
    }
    */

    CloseHandle(hVol);
    return EXIT_SUCCESS;
}