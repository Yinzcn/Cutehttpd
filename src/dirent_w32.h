
#include <windows.h>


struct dirent
{
    char    d_name[MAX_PATH];
    int     d_namlen;
};


typedef struct
{
    char            f_patt[MAX_PATH + 3];
    WIN32_FIND_DATA f_data;
    HANDLE          handle;
    struct dirent   dd_dir;
} DIR;


DIR *
opendir(const char *path)
{
    char *p;
    DIR *dirp;
    dirp = malloc(sizeof(DIR));
    if (!GetFullPathName(path, MAX_PATH, dirp->f_patt, NULL)) {
        free(dirp);
        return NULL;
    }
    p = strchr(dirp->f_patt, '\0');
    if (*(p - 1) != ':' &&
        *(p - 1) != '\\') {
        *p++ = '\\';
    }
    *p++ = '*';
    *p = '\0';
    dirp->handle = FindFirstFile(dirp->f_patt, &dirp->f_data);
    if (dirp->handle == INVALID_HANDLE_VALUE) {
        free(dirp);
        return NULL;
    }
    return dirp;
}


struct dirent *
readdir(DIR *dirp)
{
    struct dirent *dire = NULL;
    if (dirp) {
        if (dirp->handle != INVALID_HANDLE_VALUE) {
            dire = &dirp->dd_dir;
            strncpy(dire->d_name, dirp->f_data.cFileName, sizeof(dire->d_name));
            dire->d_name[sizeof(dire->d_name) - 1] = '\0';
            dire->d_namlen = strlen(dire->d_name);
            if (!FindNextFile(dirp->handle, &dirp->f_data)) {
                FindClose(dirp->handle);
                dirp->handle = INVALID_HANDLE_VALUE;
            }
        } else {
            SetLastError(ERROR_FILE_NOT_FOUND);
        }
    } else {
        SetLastError(ERROR_BAD_ARGUMENTS);
    }
    return dire;
}


int
closedir(DIR *dirp)
{
    int retn = 0;
    if (dirp == NULL) {
        return -1;
    }
    if (dirp->handle != INVALID_HANDLE_VALUE) {
        retn = FindClose(dirp->handle) ? 0 : -1;
    } else {
        retn = -1;
    }
    free(dirp);
    return retn;
}
