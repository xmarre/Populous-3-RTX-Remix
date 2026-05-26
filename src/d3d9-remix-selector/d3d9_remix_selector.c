// Populous 3 RTX Remix D3D9 selector shim.
//
// Purpose:
//   Keep NVIDIA RTX Remix out of MultiverseLauncher.exe while still allowing
//   the actual Populous game process to load the renamed Remix bridge DLL.
//
// Layout expected at runtime:
//   d3d9.dll             <- this selector shim
//   d3d9-remix.dll       <- NVIDIA RTX Remix root bridge d3d9.dll, renamed
//   .trex\               <- NVIDIA RTX Remix runtime folder
//
// This file is intentionally CRT-free and import-free so it can be built as a
// small 32-bit Windows DLL without needing the Windows SDK import libraries.

#ifndef NULL
#define NULL ((void*)0)
#endif

#define DLL_PROCESS_ATTACH 1
#define MAX_PATH_CHARS 520

typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned short     WCHAR;
typedef unsigned int       UINT;
typedef unsigned long      DWORD;
typedef long               LONG;
typedef int                BOOL;
typedef void*              PVOID;
typedef void*              HMODULE;
typedef void*              HINSTANCE;
typedef void*              FARPROC;
typedef void*              HANDLE;
typedef const char*        LPCSTR;
typedef WCHAR*             LPWSTR;
typedef const WCHAR*       LPCWSTR;

#define TRUE 1
#define FALSE 0

typedef HMODULE (__attribute__((stdcall)) *PFN_LoadLibraryW)(LPCWSTR);
typedef FARPROC (__attribute__((stdcall)) *PFN_GetProcAddress)(HMODULE, LPCSTR);
typedef DWORD   (__attribute__((stdcall)) *PFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
typedef UINT    (__attribute__((stdcall)) *PFN_GetSystemDirectoryW)(LPWSTR, UINT);
typedef UINT    (__attribute__((stdcall)) *PFN_GetPrivateProfileIntW)(LPCWSTR, LPCWSTR, UINT, LPCWSTR);
typedef void    (__attribute__((stdcall)) *PFN_RaiseException)(DWORD, DWORD, DWORD, const PVOID*);
typedef HANDLE  (__attribute__((stdcall)) *PFN_CreateFileW)(LPCWSTR, DWORD, DWORD, PVOID, DWORD, DWORD, HANDLE);
typedef BOOL    (__attribute__((stdcall)) *PFN_WriteFile)(HANDLE, const void*, DWORD, DWORD*, PVOID);
typedef BOOL    (__attribute__((stdcall)) *PFN_CloseHandle)(HANDLE);

typedef struct LIST_ENTRY_ {
  struct LIST_ENTRY_* Flink;
  struct LIST_ENTRY_* Blink;
} LIST_ENTRY;

typedef struct UNICODE_STRING_ {
  WORD Length;
  WORD MaximumLength;
  WCHAR* Buffer;
} UNICODE_STRING;

typedef struct PEB_LDR_DATA_ {
  DWORD Length;
  BYTE Initialized;
  BYTE Reserved1[3];
  PVOID SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA;

typedef struct LDR_DATA_TABLE_ENTRY_ {
  LIST_ENTRY InLoadOrderLinks;
  LIST_ENTRY InMemoryOrderLinks;
  LIST_ENTRY InInitializationOrderLinks;
  PVOID DllBase;
  PVOID EntryPoint;
  DWORD SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY;

typedef struct PEB_ {
  BYTE Reserved1[12];
  PEB_LDR_DATA* Ldr;
} PEB;

typedef struct IMAGE_DOS_HEADER_ {
  WORD e_magic;
  WORD e_cblp;
  WORD e_cp;
  WORD e_crlc;
  WORD e_cparhdr;
  WORD e_minalloc;
  WORD e_maxalloc;
  WORD e_ss;
  WORD e_sp;
  WORD e_csum;
  WORD e_ip;
  WORD e_cs;
  WORD e_lfarlc;
  WORD e_ovno;
  WORD e_res[4];
  WORD e_oemid;
  WORD e_oeminfo;
  WORD e_res2[10];
  LONG e_lfanew;
} IMAGE_DOS_HEADER;

typedef struct IMAGE_FILE_HEADER_ {
  WORD Machine;
  WORD NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD SizeOfOptionalHeader;
  WORD Characteristics;
} IMAGE_FILE_HEADER;

typedef struct IMAGE_DATA_DIRECTORY_ {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY;

typedef struct IMAGE_OPTIONAL_HEADER32_ {
  WORD Magic;
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  DWORD BaseOfData;
  DWORD ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  DWORD SizeOfStackReserve;
  DWORD SizeOfStackCommit;
  DWORD SizeOfHeapReserve;
  DWORD SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER32;

typedef struct IMAGE_NT_HEADERS32_ {
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32;

typedef struct IMAGE_EXPORT_DIRECTORY_ {
  DWORD Characteristics;
  DWORD TimeDateStamp;
  WORD MajorVersion;
  WORD MinorVersion;
  DWORD Name;
  DWORD Base;
  DWORD NumberOfFunctions;
  DWORD NumberOfNames;
  DWORD AddressOfFunctions;
  DWORD AddressOfNames;
  DWORD AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY;

#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE  0x00004550
#define IMAGE_DIRECTORY_ENTRY_EXPORT 0

static HINSTANCE g_self_module = NULL;
static HMODULE g_real_d3d9 = NULL;
static HMODULE g_remix_d3d9 = NULL;
static HMODULE g_system_d3d9 = NULL;
static unsigned int g_direct3d_create_calls = 0;
static int g_remix_active = 0;
static volatile long g_proxy_state_lock = 0;
static PFN_LoadLibraryW pLoadLibraryW = NULL;
static PFN_GetProcAddress pGetProcAddress = NULL;
static PFN_GetModuleFileNameW pGetModuleFileNameW = NULL;
static PFN_GetSystemDirectoryW pGetSystemDirectoryW = NULL;
static PFN_GetPrivateProfileIntW pGetPrivateProfileIntW = NULL;
static PFN_RaiseException pRaiseException = NULL;
static PFN_CreateFileW pCreateFileW = NULL;
static PFN_WriteFile pWriteFile = NULL;
static PFN_CloseHandle pCloseHandle = NULL;
static unsigned int g_last_selected_create_call = 0;

void MissingExportFailure_stub(void);

static void lock_proxy_state(void) {
  while (__sync_lock_test_and_set(&g_proxy_state_lock, 1)) {
    while (g_proxy_state_lock) {
    }
  }
}

static void unlock_proxy_state(void) {
  __sync_lock_release(&g_proxy_state_lock);
}

static PEB* get_peb(void) {
  PEB* peb;
  __asm__("movl %%fs:0x30, %0" : "=r"(peb));
  return peb;
}

static char lower_ascii(char c) {
  return (c >= 'A' && c <= 'Z') ? (char)(c + ('a' - 'A')) : c;
}

static WCHAR lower_wchar(WCHAR c) {
  return (c >= (WCHAR)'A' && c <= (WCHAR)'Z') ? (WCHAR)(c + ((WCHAR)'a' - (WCHAR)'A')) : c;
}

static int ascii_eq(const char* a, const char* b) {
  while (*a && *b) {
    if (lower_ascii(*a) != lower_ascii(*b)) return 0;
    ++a;
    ++b;
  }
  return *a == *b;
}

static int wide_basename_eq(const WCHAR* a, const WCHAR* b) {
  while (*a && *b) {
    if (lower_wchar(*a) != lower_wchar(*b)) return 0;
    ++a;
    ++b;
  }
  return *a == 0 && *b == 0;
}

static int wide_basename_eq_len(const UNICODE_STRING* a, const WCHAR* b) {
  unsigned int a_len;
  unsigned int i = 0;
  if (!a || !a->Buffer || !b) return 0;
  a_len = a->Length / 2;
  while (i < a_len && b[i]) {
    if (lower_wchar(a->Buffer[i]) != lower_wchar(b[i])) return 0;
    ++i;
  }
  return i == a_len && b[i] == 0;
}

static int export_name_eq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) return 0;
    ++a;
    ++b;
  }
  return *a == *b;
}

static int forwarder_module_eq(const UNICODE_STRING* wide, const char* ansi, unsigned int ansi_len) {
  unsigned int wide_len = wide->Length / 2;
  unsigned int i;
  if (!wide->Buffer) return 0;
  if (wide_len < ansi_len) return 0;
  for (i = 0; i < ansi_len; ++i) {
    if (lower_wchar(wide->Buffer[i]) != (WCHAR)lower_ascii(ansi[i])) return 0;
  }
  if (wide_len == ansi_len) return 1;
  if (wide_len == ansi_len + 4 &&
      wide->Buffer[ansi_len + 0] == (WCHAR)'.' &&
      lower_wchar(wide->Buffer[ansi_len + 1]) == (WCHAR)'d' &&
      lower_wchar(wide->Buffer[ansi_len + 2]) == (WCHAR)'l' &&
      lower_wchar(wide->Buffer[ansi_len + 3]) == (WCHAR)'l') {
    return 1;
  }
  return 0;
}

static HMODULE find_loaded_module_w(const WCHAR* basename) {
  PEB* peb = get_peb();
  LIST_ENTRY* head;
  LIST_ENTRY* cur;
  if (!peb || !peb->Ldr) return NULL;
  head = &peb->Ldr->InLoadOrderModuleList;
  cur = head->Flink;
  while (cur && cur != head) {
    LDR_DATA_TABLE_ENTRY* ent = (LDR_DATA_TABLE_ENTRY*)cur;
    if (wide_basename_eq_len(&ent->BaseDllName, basename)) {
      return (HMODULE)ent->DllBase;
    }
    cur = cur->Flink;
  }
  return NULL;
}

static HMODULE find_loaded_module_forwarder(const char* module_name, unsigned int module_len) {
  PEB* peb = get_peb();
  LIST_ENTRY* head;
  LIST_ENTRY* cur;
  if (!peb || !peb->Ldr) return NULL;
  head = &peb->Ldr->InLoadOrderModuleList;
  cur = head->Flink;
  while (cur && cur != head) {
    LDR_DATA_TABLE_ENTRY* ent = (LDR_DATA_TABLE_ENTRY*)cur;
    if (forwarder_module_eq(&ent->BaseDllName, module_name, module_len)) {
      return (HMODULE)ent->DllBase;
    }
    cur = cur->Flink;
  }
  return NULL;
}

static FARPROC resolve_export_raw(HMODULE module, const char* name, int allow_forwarder);

static FARPROC resolve_forwarder(const char* forwarder) {
  const char* dot = forwarder;
  char proc_name[128];
  unsigned int module_len = 0;
  unsigned int proc_len = 0;
  HMODULE module;

  while (*dot && *dot != '.') ++dot;
  if (*dot != '.') return NULL;
  module_len = (unsigned int)(dot - forwarder);
  if (module_len == 0) return NULL;

  module = find_loaded_module_forwarder(forwarder, module_len);
  if (!module) return NULL;

  ++dot;
  while (dot[proc_len] && proc_len + 1 < sizeof(proc_name)) {
    proc_name[proc_len] = dot[proc_len];
    ++proc_len;
  }
  proc_name[proc_len] = 0;
  if (proc_len == 0) return NULL;

  return resolve_export_raw(module, proc_name, 0);
}

static FARPROC resolve_export_raw(HMODULE module, const char* name, int allow_forwarder) {
  BYTE* base = (BYTE*)module;
  IMAGE_DOS_HEADER* dos;
  IMAGE_NT_HEADERS32* nt;
  IMAGE_DATA_DIRECTORY* dir;
  IMAGE_EXPORT_DIRECTORY* exp;
  DWORD* names;
  WORD* ordinals;
  DWORD* functions;
  DWORD i;

  if (!base) return NULL;
  dos = (IMAGE_DOS_HEADER*)base;
  if (dos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
  nt = (IMAGE_NT_HEADERS32*)(base + dos->e_lfanew);
  if (nt->Signature != IMAGE_NT_SIGNATURE) return NULL;
  dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (!dir->VirtualAddress || !dir->Size) return NULL;

  exp = (IMAGE_EXPORT_DIRECTORY*)(base + dir->VirtualAddress);
  names = (DWORD*)(base + exp->AddressOfNames);
  ordinals = (WORD*)(base + exp->AddressOfNameOrdinals);
  functions = (DWORD*)(base + exp->AddressOfFunctions);

  for (i = 0; i < exp->NumberOfNames; ++i) {
    const char* export_name = (const char*)(base + names[i]);
    if (export_name_eq(export_name, name)) {
      DWORD rva = functions[ordinals[i]];
      if (allow_forwarder && rva >= dir->VirtualAddress && rva < dir->VirtualAddress + dir->Size) {
        return resolve_forwarder((const char*)(base + rva));
      }
      return (FARPROC)(base + rva);
    }
  }
  return NULL;
}

static FARPROC resolve_export(HMODULE module, const char* name) {
  return resolve_export_raw(module, name, 1);
}

static int ensure_kernel_functions(void) {
  static const WCHAR KERNEL32_DLL[] = { 'k','e','r','n','e','l','3','2','.','d','l','l',0 };
  static const WCHAR KERNELBASE_DLL[] = { 'k','e','r','n','e','l','b','a','s','e','.','d','l','l',0 };
  HMODULE k32;
  HMODULE kb;

  if (pLoadLibraryW && pGetProcAddress && pGetModuleFileNameW && pGetSystemDirectoryW && pGetPrivateProfileIntW && pRaiseException && pCreateFileW && pWriteFile && pCloseHandle) return 1;

  k32 = find_loaded_module_w(KERNEL32_DLL);
  kb = find_loaded_module_w(KERNELBASE_DLL);

  if (!pGetProcAddress) {
    pGetProcAddress = (PFN_GetProcAddress)resolve_export(k32, "GetProcAddress");
    if (!pGetProcAddress) pGetProcAddress = (PFN_GetProcAddress)resolve_export(kb, "GetProcAddress");
  }
  if (!pLoadLibraryW) {
    pLoadLibraryW = (PFN_LoadLibraryW)resolve_export(k32, "LoadLibraryW");
    if (!pLoadLibraryW) pLoadLibraryW = (PFN_LoadLibraryW)resolve_export(kb, "LoadLibraryW");
  }
  if (!pGetModuleFileNameW) {
    pGetModuleFileNameW = (PFN_GetModuleFileNameW)resolve_export(k32, "GetModuleFileNameW");
    if (!pGetModuleFileNameW) pGetModuleFileNameW = (PFN_GetModuleFileNameW)resolve_export(kb, "GetModuleFileNameW");
  }
  if (!pGetSystemDirectoryW) {
    pGetSystemDirectoryW = (PFN_GetSystemDirectoryW)resolve_export(k32, "GetSystemDirectoryW");
    if (!pGetSystemDirectoryW) pGetSystemDirectoryW = (PFN_GetSystemDirectoryW)resolve_export(kb, "GetSystemDirectoryW");
  }
  if (!pGetPrivateProfileIntW) {
    pGetPrivateProfileIntW = (PFN_GetPrivateProfileIntW)resolve_export(k32, "GetPrivateProfileIntW");
    if (!pGetPrivateProfileIntW) pGetPrivateProfileIntW = (PFN_GetPrivateProfileIntW)resolve_export(kb, "GetPrivateProfileIntW");
  }
  if (!pRaiseException) {
    pRaiseException = (PFN_RaiseException)resolve_export(k32, "RaiseException");
    if (!pRaiseException) pRaiseException = (PFN_RaiseException)resolve_export(kb, "RaiseException");
  }
  if (!pCreateFileW) {
    pCreateFileW = (PFN_CreateFileW)resolve_export(k32, "CreateFileW");
    if (!pCreateFileW) pCreateFileW = (PFN_CreateFileW)resolve_export(kb, "CreateFileW");
  }
  if (!pWriteFile) {
    pWriteFile = (PFN_WriteFile)resolve_export(k32, "WriteFile");
    if (!pWriteFile) pWriteFile = (PFN_WriteFile)resolve_export(kb, "WriteFile");
  }
  if (!pCloseHandle) {
    pCloseHandle = (PFN_CloseHandle)resolve_export(k32, "CloseHandle");
    if (!pCloseHandle) pCloseHandle = (PFN_CloseHandle)resolve_export(kb, "CloseHandle");
  }

  return pLoadLibraryW && pGetProcAddress && pGetModuleFileNameW && pGetSystemDirectoryW && pGetPrivateProfileIntW && pRaiseException && pCreateFileW && pWriteFile && pCloseHandle;
}

static unsigned int wide_len(const WCHAR* s) {
  unsigned int n = 0;
  if (!s) return 0;
  while (s[n]) ++n;
  return n;
}

static void wide_append(WCHAR* dst, unsigned int cap, const WCHAR* suffix) {
  unsigned int n = wide_len(dst);
  unsigned int i = 0;
  if (!cap || n >= cap) return;
  while (suffix && suffix[i] && n + i + 1 < cap) {
    dst[n + i] = suffix[i];
    ++i;
  }
  dst[n + i] = 0;
}

static void directory_from_path(WCHAR* path) {
  int i = (int)wide_len(path) - 1;
  while (i >= 0) {
    if (path[i] == (WCHAR)'\\' || path[i] == (WCHAR)'/') {
      path[i] = 0;
      return;
    }
    --i;
  }
  path[0] = 0;
}

static const WCHAR* basename_from_path(const WCHAR* path) {
  const WCHAR* base = path;
  unsigned int i = 0;
  while (path && path[i]) {
    if (path[i] == (WCHAR)'\\' || path[i] == (WCHAR)'/') base = path + i + 1;
    ++i;
  }
  return base;
}

static int get_module_file_name_checked(HMODULE module, WCHAR* dst, DWORD cap) {
  DWORD len;
  if (!dst || cap < 2) return 0;
  dst[0] = 0;
  if (!ensure_kernel_functions()) return 0;
  len = pGetModuleFileNameW(module, dst, cap);
  if (!len || len >= cap - 1) {
    dst[cap - 1] = 0;
    return 0;
  }
  dst[len] = 0;
  return 1;
}

static int get_process_basename(WCHAR* exe_name, DWORD cap) {
  WCHAR exe_path[MAX_PATH_CHARS];
  const WCHAR* base;
  unsigned int i = 0;

  if (!exe_name || cap < 2) return 0;
  exe_name[0] = 0;

  exe_path[0] = 0;
  if (!get_module_file_name_checked(NULL, exe_path, MAX_PATH_CHARS)) return 0;
  base = basename_from_path(exe_path);

  while (base[i] && i + 1 < cap) {
    exe_name[i] = base[i];
    ++i;
  }
  exe_name[i] = 0;
  return i > 0;
}

static int is_game_process(void) {
  static const WCHAR POPTBM_EXE[]   = { 'p','o','p','T','B','M','.','e','x','e',0 };
  static const WCHAR D3DPOPTB_EXE[] = { 'D','3','D','P','o','p','T','B','.','e','x','e',0 };
  static const WCHAR POPTB_EXE[]    = { 'p','o','p','T','B','.','e','x','e',0 };
  WCHAR exe_name[MAX_PATH_CHARS];

  if (!get_process_basename(exe_name, MAX_PATH_CHARS)) return 0;

  return wide_basename_eq(exe_name, POPTBM_EXE) ||
         wide_basename_eq(exe_name, D3DPOPTB_EXE) ||
         wide_basename_eq(exe_name, POPTB_EXE);
}

static int is_multiverse_game_process(void) {
  static const WCHAR POPTBM_EXE[] = { 'p','o','p','T','B','M','.','e','x','e',0 };
  WCHAR exe_name[MAX_PATH_CHARS];

  if (!get_process_basename(exe_name, MAX_PATH_CHARS)) return 0;
  return wide_basename_eq(exe_name, POPTBM_EXE);
}

static HMODULE load_system_d3d9(void) {
  static const WCHAR SLASH_D3D9_DLL[] = { '\\','d','3','d','9','.','d','l','l',0 };
  WCHAR path[MAX_PATH_CHARS];
  UINT len;
  path[0] = 0;
  if (!ensure_kernel_functions()) return NULL;
  len = pGetSystemDirectoryW(path, MAX_PATH_CHARS);
  if (!len || len >= MAX_PATH_CHARS) return NULL;
  path[len] = 0;
  wide_append(path, MAX_PATH_CHARS, SLASH_D3D9_DLL);
  return pLoadLibraryW(path);
}

static HMODULE ensure_system_d3d9(void) {
  HMODULE mod;

  lock_proxy_state();
  mod = g_system_d3d9;
  unlock_proxy_state();
  if (mod) return mod;

  mod = load_system_d3d9();

  lock_proxy_state();
  if (!g_system_d3d9) g_system_d3d9 = mod;
  mod = g_system_d3d9;
  unlock_proxy_state();
  return mod;
}

static HMODULE load_remix_bridge(void) {
  static const WCHAR REMIX_BRIDGE_NAME[] = { '\\','d','3','d','9','-','r','e','m','i','x','.','d','l','l',0 };
  WCHAR path[MAX_PATH_CHARS];
  path[0] = 0;
  if (!get_module_file_name_checked((HMODULE)g_self_module, path, MAX_PATH_CHARS)) return NULL;
  directory_from_path(path);
  wide_append(path, MAX_PATH_CHARS, REMIX_BRIDGE_NAME);
  return pLoadLibraryW(path);
}

static HMODULE ensure_remix_d3d9(void) {
  HMODULE mod;

  lock_proxy_state();
  mod = g_remix_d3d9;
  unlock_proxy_state();
  if (mod) return mod;

  mod = load_remix_bridge();

  lock_proxy_state();
  if (!g_remix_d3d9) g_remix_d3d9 = mod;
  mod = g_remix_d3d9;
  unlock_proxy_state();
  return mod;
}


#define FILE_APPEND_DATA      0x00000004u
#define FILE_SHARE_READ       0x00000001u
#define FILE_SHARE_WRITE      0x00000002u
#define OPEN_ALWAYS           4u
#define FILE_ATTRIBUTE_NORMAL 0x00000080u
#define INVALID_HANDLE_VALUE_LOCAL ((HANDLE)(LONG)-1)

static unsigned int ascii_len_local(const char* s) {
  unsigned int n = 0;
  if (!s) return 0;
  while (s[n]) ++n;
  return n;
}

static int get_selector_log_path(WCHAR* path, DWORD cap) {
  static const WCHAR LOG_NAME[] = { '\\','d','3','d','9','-','s','e','l','e','c','t','o','r','.','l','o','g',0 };
  if (!path || cap < 2) return 0;
  path[0] = 0;
  if (!get_module_file_name_checked((HMODULE)g_self_module, path, cap)) return 0;
  directory_from_path(path);
  wide_append(path, cap, LOG_NAME);
  return path[0] != 0;
}

static void log_write_raw(const char* text, unsigned int len) {
  WCHAR path[MAX_PATH_CHARS];
  HANDLE h;
  DWORD written = 0;
  if (!text || !len || !ensure_kernel_functions() || !pCreateFileW || !pWriteFile || !pCloseHandle) return;
  if (!get_selector_log_path(path, MAX_PATH_CHARS)) return;
  h = pCreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (!h || h == INVALID_HANDLE_VALUE_LOCAL) return;
  pWriteFile(h, text, (DWORD)len, &written, NULL);
  pCloseHandle(h);
}

static void log_write_text(const char* text) {
  log_write_raw(text, ascii_len_local(text));
}

static void log_write_uint(UINT value) {
  char buf[16];
  unsigned int i = 0;
  unsigned int j;
  if (value == 0) {
    log_write_raw("0", 1);
    return;
  }
  while (value && i < sizeof(buf)) {
    buf[i++] = (char)('0' + (value % 10));
    value /= 10;
  }
  for (j = 0; j < i / 2; ++j) {
    char t = buf[j];
    buf[j] = buf[i - 1 - j];
    buf[i - 1 - j] = t;
  }
  log_write_raw(buf, i);
}

static void log_write_hex32(DWORD value) {
  static const char HEX[] = "0123456789ABCDEF";
  char buf[10];
  int i;
  buf[0] = '0';
  buf[1] = 'x';
  for (i = 0; i < 8; ++i) {
    buf[2 + i] = HEX[(value >> (28 - i * 4)) & 0xF];
  }
  log_write_raw(buf, 10);
}

static void log_create_selection(UINT create_call, const char* api, const char* backend, UINT defer_count) {
  log_write_text("selector: ");
  log_write_text(api);
  log_write_text(" call=");
  log_write_uint(create_call);
  log_write_text(" deferCreates=");
  log_write_uint(defer_count);
  log_write_text(" backend=");
  log_write_text(backend);
  log_write_text("\r\n");
}



static void log_backend_promotion(UINT create_call, const char* reason, UINT ok) {
  log_write_text("selector: promote d3dCreateCall=");
  log_write_uint(create_call);
  log_write_text(" reason=");
  log_write_text(reason);
  log_write_text(" result=");
  log_write_text(ok ? "remix" : "failed");
  log_write_text("\r\n");
}

static int get_selector_ini_path(WCHAR* path, DWORD cap) {
  static const WCHAR INI_NAME[] = { '\\','d','3','d','9','-','s','e','l','e','c','t','o','r','.','i','n','i',0 };
  if (!path || cap < 2) return 0;
  path[0] = 0;
  if (!get_module_file_name_checked((HMODULE)g_self_module, path, cap)) return 0;
  directory_from_path(path);
  wide_append(path, cap, INI_NAME);
  return path[0] != 0;
}

static UINT read_selector_ini_uint(const WCHAR* section, const WCHAR* key, UINT fallback, UINT max_value) {
  WCHAR path[MAX_PATH_CHARS];
  UINT value;
  if (!ensure_kernel_functions() || !pGetPrivateProfileIntW) return fallback;
  if (!get_selector_ini_path(path, MAX_PATH_CHARS)) return fallback;
  value = pGetPrivateProfileIntW(section, key, fallback, path);
  if (value > max_value) value = max_value;
  return value;
}

static UINT multiverse_defer_create_count(void) {
  static const WCHAR SECTION[] = { 'p','o','p','T','B','M',0 };
  static const WCHAR KEY[] = { 'd','e','f','e','r','C','r','e','a','t','e','s',0 };
  UINT value = read_selector_ini_uint(SECTION, KEY, 3, 16);
  if (value < 2) value = 2;
  return value;
}

static UINT poptbm_force_windowed_for_remix(void) {
  static const WCHAR SECTION[] = { 'p','o','p','T','B','M',0 };
  static const WCHAR KEY[] = { 'f','o','r','c','e','W','i','n','d','o','w','e','d','F','o','r','R','e','m','i','x',0 };
  return read_selector_ini_uint(SECTION, KEY, 1, 1);
}

static UINT poptbm_enable_rhw_fixup(void) {
  static const WCHAR SECTION[] = { 'p','o','p','T','B','M',0 };
  static const WCHAR KEY[] = { 'e','n','a','b','l','e','R','h','w','F','i','x','u','p',0 };
  return read_selector_ini_uint(SECTION, KEY, 0, 1);
}

static UINT poptbm_promote_system_device_with_auto_depth(void) {
  static const WCHAR SECTION[] = { 'p','o','p','T','B','M',0 };
  static const WCHAR KEY[] = { 'p','r','o','m','o','t','e','S','y','s','t','e','m','D','e','v','i','c','e','W','i','t','h','A','u','t','o','D','e','p','t','h',0 };
  return read_selector_ini_uint(SECTION, KEY, 1, 1);
}

static HMODULE ensure_real_d3d9(void) {
  HMODULE mod;
  int remix_active;

  lock_proxy_state();
  mod = g_real_d3d9;
  remix_active = g_remix_active;
  unlock_proxy_state();
  if (mod) return mod;

  if (is_game_process() && remix_active) {
    mod = ensure_remix_d3d9();
    if (!mod) mod = ensure_system_d3d9();
  } else if (is_game_process() && !is_multiverse_game_process()) {
    mod = ensure_remix_d3d9();
    if (!mod) mod = ensure_system_d3d9();
  } else {
    mod = ensure_system_d3d9();
  }

  lock_proxy_state();
  if (!g_real_d3d9 || mod == g_remix_d3d9) g_real_d3d9 = mod;
  mod = g_real_d3d9;
  unlock_proxy_state();
  return mod;
}

static HMODULE select_d3d9_for_create(const char* api_name) {
  HMODULE mod;
  unsigned int create_call;
  UINT defer_count;
  int is_multiverse_game = is_multiverse_game_process();
  int is_game = is_game_process();

  lock_proxy_state();
  create_call = ++g_direct3d_create_calls;
  g_last_selected_create_call = create_call;
  unlock_proxy_state();

  defer_count = multiverse_defer_create_count();
  if (is_multiverse_game && create_call <= defer_count) {
    log_create_selection(create_call, api_name, "system", defer_count);
    return ensure_system_d3d9();
  }

  if (is_game) {
    mod = ensure_remix_d3d9();
    if (mod) {
      lock_proxy_state();
      g_remix_active = 1;
      g_real_d3d9 = mod;
      unlock_proxy_state();
      log_create_selection(create_call, api_name, "remix", defer_count);
      return mod;
    }
  }

  log_create_selection(create_call, api_name, "system-fallback", defer_count);
  return ensure_system_d3d9();
}

typedef struct EXPORT_CACHE_ {
  const char* name;
  unsigned int ordinal;
  FARPROC proc;
} EXPORT_CACHE;

static EXPORT_CACHE g_exports[] = {
  { NULL, 16, NULL },
  { NULL, 17, NULL },
  { NULL, 18, NULL },
  { NULL, 19, NULL },
  { "Direct3DCreate9On12", 20, NULL },
  { "Direct3DCreate9On12Ex", 21, NULL },
  { NULL, 22, NULL },
  { NULL, 23, NULL },
  { "Direct3DShaderValidatorCreate9", 24, NULL },
  { "PSGPError", 25, NULL },
  { "PSGPSampleTexture", 26, NULL },
  { "D3DPERF_BeginEvent", 27, NULL },
  { "D3DPERF_EndEvent", 28, NULL },
  { "D3DPERF_GetStatus", 29, NULL },
  { "D3DPERF_QueryRepeatFrame", 30, NULL },
  { "D3DPERF_SetMarker", 31, NULL },
  { "D3DPERF_SetOptions", 32, NULL },
  { "D3DPERF_SetRegion", 33, NULL },
  { "DebugSetLevel", 34, NULL },
  { "DebugSetMute", 35, NULL },
  { "Direct3D9EnableMaximizedWindowedModeShim", 36, NULL },
  { "Direct3DCreate9", 37, NULL },
  { "Direct3DCreate9Ex", 38, NULL },
  { NULL, 0, NULL }
};

static FARPROC get_proc_by_name_or_ordinal(HMODULE mod, const char* name, unsigned int ordinal) {
  FARPROC proc = NULL;
  if (!mod || !ensure_kernel_functions()) return NULL;
  if (name) proc = pGetProcAddress(mod, name);
  if (!proc && ordinal) proc = pGetProcAddress(mod, (LPCSTR)(DWORD)ordinal);
  return proc;
}

static FARPROC resolve_export_entry(EXPORT_CACHE* entry) {
  HMODULE selected;
  HMODULE system;
  FARPROC proc;

  if (entry->proc) return entry->proc;

  selected = ensure_real_d3d9();
  proc = get_proc_by_name_or_ordinal(selected, entry->name, entry->ordinal);
  if (!proc) {
    system = ensure_system_d3d9();
    if (system != selected) {
      proc = get_proc_by_name_or_ordinal(system, entry->name, entry->ordinal);
    }
  }

  if (!proc) proc = (FARPROC)MissingExportFailure_stub;
  entry->proc = proc;
  return proc;
}

void* ResolveExportByName(const char* name) {
  unsigned int i;
  for (i = 0; g_exports[i].name || g_exports[i].ordinal; ++i) {
    if (g_exports[i].name && ascii_eq(g_exports[i].name, name)) {
      return resolve_export_entry(&g_exports[i]);
    }
  }
  return NULL;
}

void* ResolveExportByOrdinal(unsigned int ordinal) {
  unsigned int i;
  for (i = 0; g_exports[i].name || g_exports[i].ordinal; ++i) {
    if (g_exports[i].ordinal == ordinal) {
      return resolve_export_entry(&g_exports[i]);
    }
  }
  return NULL;
}

static void __attribute__((used)) RaiseMissingExport(void) {
  if (ensure_kernel_functions() && pRaiseException) {
    pRaiseException(0xC0000139, 0, 0, NULL);
  }
}

__attribute__((naked)) void MissingExportFailure_stub(void) {
  __asm__("call _RaiseMissingExport\n\t"
          "ud2\n");
}



/*
 * Populous D3D9 compatibility layer.
 *
 * Remix is loaded through the renamed NVIDIA bridge only in the real game
 * process. Multiverse's Direct3D9 renderer creates and tears down multiple
 * exclusive fullscreen devices around the intro/menu transition. The bridge
 * log shows this as a successful fullscreen CreateDevice followed immediately
 * by resource/swapchain/module destruction and a 64-bit bridge access
 * violation.
 *
 * The root fix here is to keep Remix out of exclusive fullscreen ownership for
 * popTBM.exe. The D3D9 object is proxied only so CreateDevice/CreateDeviceEx can
 * pass windowed presentation parameters to the Remix bridge. The experimental
 * RHW vertex fixup remains compiled in, but is disabled by default because the
 * current crash occurs before that path is relevant.
 */

#define S_OK 0
#define E_NOINTERFACE ((HRESULT)0x80004002u)
#define E_POINTER     ((HRESULT)0x80004003u)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

typedef LONG HRESULT;
typedef unsigned long ULONG;
typedef unsigned long D3DCOLOR;
typedef unsigned int D3DFORMAT;
typedef unsigned int D3DDEVTYPE;
typedef int INT;
typedef void* HWND;
typedef struct IDirect3D9_ IDirect3D9;
typedef struct IDirect3DDevice9_ IDirect3DDevice9;
typedef struct IDirect3DVertexDeclaration9_ IDirect3DVertexDeclaration9;
typedef struct IDirect3DVertexBuffer9_ IDirect3DVertexBuffer9;

typedef struct GUID_ {
  DWORD Data1;
  WORD Data2;
  WORD Data3;
  BYTE Data4[8];
} GUID;

typedef struct D3DDISPLAYMODE_ {
  UINT Width;
  UINT Height;
  UINT RefreshRate;
  D3DFORMAT Format;
} D3DDISPLAYMODE;

typedef struct D3DDISPLAYMODEEX_ {
  UINT Size;
  UINT Width;
  UINT Height;
  UINT RefreshRate;
  D3DFORMAT Format;
  UINT ScanLineOrdering;
} D3DDISPLAYMODEEX;

typedef struct D3DPRESENT_PARAMETERS_ {
  UINT BackBufferWidth;
  UINT BackBufferHeight;
  D3DFORMAT BackBufferFormat;
  UINT BackBufferCount;
  UINT MultiSampleType;
  DWORD MultiSampleQuality;
  UINT SwapEffect;
  HWND hDeviceWindow;
  BOOL Windowed;
  BOOL EnableAutoDepthStencil;
  D3DFORMAT AutoDepthStencilFormat;
  DWORD Flags;
  UINT FullScreen_RefreshRateInHz;
  UINT PresentationInterval;
} D3DPRESENT_PARAMETERS;

static void log_device_create(const char* api, const char* backend, UINT create_call, const D3DPRESENT_PARAMETERS* pp, DWORD behavior, HRESULT hr, UINT forced_windowed) {
  log_write_text("selector: ");
  log_write_text(api);
  log_write_text(" d3dCreateCall=");
  log_write_uint(create_call);
  log_write_text(" backend=");
  log_write_text(backend);
  log_write_text(" forcedWindowed=");
  log_write_uint(forced_windowed);
  log_write_text(" behavior=");
  log_write_hex32(behavior);
  if (pp) {
    log_write_text(" w=");
    log_write_uint(pp->BackBufferWidth);
    log_write_text(" h=");
    log_write_uint(pp->BackBufferHeight);
    log_write_text(" windowed=");
    log_write_uint((UINT)pp->Windowed);
    log_write_text(" depth=");
    log_write_uint((UINT)pp->EnableAutoDepthStencil);
    log_write_text(" refresh=");
    log_write_uint(pp->FullScreen_RefreshRateInHz);
  }
  log_write_text(" hr=");
  log_write_hex32((DWORD)hr);
  log_write_text("\r\n");
}



typedef struct D3DVIEWPORT9_ {
  DWORD X;
  DWORD Y;
  DWORD Width;
  DWORD Height;
  float MinZ;
  float MaxZ;
} D3DVIEWPORT9;

typedef struct D3DMATRIX_ {
  union {
    struct {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4];
  };
} D3DMATRIX;

typedef struct D3DVERTEXELEMENT9_ {
  WORD Stream;
  WORD Offset;
  BYTE Type;
  BYTE Method;
  BYTE Usage;
  BYTE UsageIndex;
} D3DVERTEXELEMENT9;

#define D3DTS_VIEW       2
#define D3DTS_PROJECTION 3
#define D3DTS_WORLD      256

#define D3DFVF_RESERVED0      0x001u
#define D3DFVF_XYZ            0x002u
#define D3DFVF_XYZRHW         0x004u
#define D3DFVF_NORMAL         0x010u
#define D3DFVF_PSIZE          0x020u
#define D3DFVF_DIFFUSE        0x040u
#define D3DFVF_SPECULAR       0x080u
#define D3DFVF_TEXCOUNT_MASK  0xF00u
#define D3DFVF_TEXCOUNT_SHIFT 8

#define D3DDECLTYPE_FLOAT1   0
#define D3DDECLTYPE_FLOAT2   1
#define D3DDECLTYPE_FLOAT3   2
#define D3DDECLTYPE_FLOAT4   3
#define D3DDECLTYPE_D3DCOLOR 4
#define D3DDECLTYPE_UNUSED   17
#define D3DDECLMETHOD_DEFAULT 0
#define D3DDECLUSAGE_POSITION 0
#define D3DDECLUSAGE_PSIZE    4
#define D3DDECLUSAGE_TEXCOORD 5
#define D3DDECLUSAGE_COLOR    10

#define D3DPT_POINTLIST     1
#define D3DPT_LINELIST      2
#define D3DPT_LINESTRIP     3
#define D3DPT_TRIANGLELIST  4
#define D3DPT_TRIANGLESTRIP 5
#define D3DPT_TRIANGLEFAN   6

#define MAX_D3D9_PROXIES 8
#define MAX_DEVICE_PROXIES 16

#define PROXY_MAGIC_D3D9 0x44523950u
#define PROXY_MAGIC_DEV  0x56453950u

typedef HRESULT (__attribute__((stdcall)) *PFN_Direct3DCreate9Ex_Real)(UINT, void**);
typedef void*   (__attribute__((stdcall)) *PFN_Direct3DCreate9_Real)(UINT);

typedef struct D3D9Proxy_ {
  void** lpVtbl;
  void* real;
  DWORD magic;
  int is_ex;
  int is_remix_backend;
  UINT sdk_version;
  unsigned int create_call_index;
} D3D9Proxy;

typedef struct DeviceProxy_ {
  void** lpVtbl;
  void* real;
  DWORD magic;
  D3D9Proxy* parent;
  int is_ex;
  DWORD game_fvf;
  DWORD active_fvf;
  int rhw_active;
  int using_game_decl;
  int vertex_shader_active;
  UINT bb_width;
  UINT bb_height;
  D3DVIEWPORT9 viewport;
  IDirect3DVertexDeclaration9* rhw_decl;
  DWORD rhw_decl_fvf;
  unsigned int create_call_index;
} DeviceProxy;

static D3D9Proxy g_d3d9_proxy_pool[MAX_D3D9_PROXIES];
static DeviceProxy g_device_proxy_pool[MAX_DEVICE_PROXIES];
static void* g_d3d9_vtbl[22];
static void* g_device_vtbl[134];
static int g_proxy_vtables_init = 0;

static int guid_eq(const GUID* a, const GUID* b) {
  unsigned int i;
  if (!a || !b) return 0;
  if (a->Data1 != b->Data1 || a->Data2 != b->Data2 || a->Data3 != b->Data3) return 0;
  for (i = 0; i < 8; ++i) if (a->Data4[i] != b->Data4[i]) return 0;
  return 1;
}

static const GUID IID_IUnknown_local = {0x00000000u,0x0000u,0x0000u,{0xC0u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x46u}};
static const GUID IID_IDirect3D9_local = {0x81BDCBCAu,0x64D4u,0x426Du,{0xAEu,0x8Du,0xADu,0x01u,0x47u,0xF4u,0x27u,0x5Cu}};
static const GUID IID_IDirect3D9Ex_local = {0x02177241u,0x69FCu,0x400Cu,{0x8Fu,0xF1u,0x93u,0xA4u,0x4Du,0xF6u,0x86u,0x1Du}};
static const GUID IID_IDirect3DDevice9_local = {0xD0223B96u,0xBF7Au,0x43FDu,{0x92u,0xBDu,0xA4u,0x3Bu,0x0Du,0x82u,0xB9u,0xEBu}};
static const GUID IID_IDirect3DDevice9Ex_local = {0xB18B10CEu,0x2649u,0x405Au,{0x87u,0x0Fu,0x95u,0xF7u,0x77u,0xD4u,0x31u,0x3Au}};

static ULONG call_addref(void* obj) {
  void** vt = obj ? *(void***)obj : NULL;
  ULONG (__attribute__((stdcall)) *fn)(void*) = vt ? (ULONG (__attribute__((stdcall)) *)(void*))vt[1] : NULL;
  return fn ? fn(obj) : 1;
}

static ULONG call_release(void* obj) {
  void** vt = obj ? *(void***)obj : NULL;
  ULONG (__attribute__((stdcall)) *fn)(void*) = vt ? (ULONG (__attribute__((stdcall)) *)(void*))vt[2] : NULL;
  return fn ? fn(obj) : 0;
}

static HRESULT call_set_transform(void* dev, DWORD state, const D3DMATRIX* mat) {
  void** vt = dev ? *(void***)dev : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*, DWORD, const D3DMATRIX*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,DWORD,const D3DMATRIX*))vt[44] : NULL;
  return fn ? fn(dev, state, mat) : E_POINTER;
}

static HRESULT call_set_vertex_declaration(void* dev, IDirect3DVertexDeclaration9* decl) {
  void** vt = dev ? *(void***)dev : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*, IDirect3DVertexDeclaration9*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,IDirect3DVertexDeclaration9*))vt[87] : NULL;
  return fn ? fn(dev, decl) : E_POINTER;
}

static HRESULT call_set_fvf(void* dev, DWORD fvf) {
  void** vt = dev ? *(void***)dev : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*, DWORD) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,DWORD))vt[89] : NULL;
  return fn ? fn(dev, fvf) : E_POINTER;
}

static HRESULT call_create_vertex_declaration(void* dev, const D3DVERTEXELEMENT9* elems, IDirect3DVertexDeclaration9** out_decl) {
  void** vt = dev ? *(void***)dev : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*, const D3DVERTEXELEMENT9*, IDirect3DVertexDeclaration9**) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,const D3DVERTEXELEMENT9*,IDirect3DVertexDeclaration9**))vt[86] : NULL;
  return fn ? fn(dev, elems, out_decl) : E_POINTER;
}

static unsigned int fvf_texcoord_dim(DWORD fvf, unsigned int index) {
  unsigned int bits = (fvf >> (16 + index * 2)) & 3u;
  if (bits == 3u) return 1;
  if (bits == 0u) return 2;
  if (bits == 1u) return 3;
  return 4;
}

static BYTE decl_float_type_for_dim(unsigned int dim) {
  if (dim <= 1) return D3DDECLTYPE_FLOAT1;
  if (dim == 2) return D3DDECLTYPE_FLOAT2;
  if (dim == 3) return D3DDECLTYPE_FLOAT3;
  return D3DDECLTYPE_FLOAT4;
}

static WORD add_vertex_element(D3DVERTEXELEMENT9* elems, unsigned int* count, WORD offset, BYTE type, BYTE usage, BYTE usage_index) {
  elems[*count].Stream = 0;
  elems[*count].Offset = offset;
  elems[*count].Type = type;
  elems[*count].Method = D3DDECLMETHOD_DEFAULT;
  elems[*count].Usage = usage;
  elems[*count].UsageIndex = usage_index;
  ++(*count);
  if (type == D3DDECLTYPE_FLOAT1) return (WORD)(offset + 4);
  if (type == D3DDECLTYPE_FLOAT2) return (WORD)(offset + 8);
  if (type == D3DDECLTYPE_FLOAT3) return (WORD)(offset + 12);
  if (type == D3DDECLTYPE_FLOAT4) return (WORD)(offset + 16);
  if (type == D3DDECLTYPE_D3DCOLOR) return (WORD)(offset + 4);
  return offset;
}

static int build_rhw_vertex_declaration(DWORD fvf, D3DVERTEXELEMENT9* elems, unsigned int max_elems) {
  unsigned int count = 0;
  WORD offset = 0;
  unsigned int tex_count;
  unsigned int i;

  if (!(fvf & D3DFVF_XYZRHW)) return 0;
  if (max_elems < 4) return 0;

  /* Input is x,y,z,rhw. Expose x,y,z as untransformed POSITION and ignore rhw. */
  elems[count].Stream = 0;
  elems[count].Offset = 0;
  elems[count].Type = D3DDECLTYPE_FLOAT3;
  elems[count].Method = D3DDECLMETHOD_DEFAULT;
  elems[count].Usage = D3DDECLUSAGE_POSITION;
  elems[count].UsageIndex = 0;
  ++count;
  offset = 16;

  if (fvf & D3DFVF_PSIZE) {
    if (count + 1 >= max_elems) return 0;
    offset = add_vertex_element(elems, &count, offset, D3DDECLTYPE_FLOAT1, D3DDECLUSAGE_PSIZE, 0);
  }
  if (fvf & D3DFVF_DIFFUSE) {
    if (count + 1 >= max_elems) return 0;
    offset = add_vertex_element(elems, &count, offset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, 0);
  }
  if (fvf & D3DFVF_SPECULAR) {
    if (count + 1 >= max_elems) return 0;
    offset = add_vertex_element(elems, &count, offset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, 1);
  }

  tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  for (i = 0; i < tex_count; ++i) {
    unsigned int dim = fvf_texcoord_dim(fvf, i);
    if (count + 1 >= max_elems) return 0;
    offset = add_vertex_element(elems, &count, offset, decl_float_type_for_dim(dim), D3DDECLUSAGE_TEXCOORD, (BYTE)i);
  }

  elems[count].Stream = 0xFF;
  elems[count].Offset = 0;
  elems[count].Type = D3DDECLTYPE_UNUSED;
  elems[count].Method = 0;
  elems[count].Usage = 0;
  elems[count].UsageIndex = 0;
  return 1;
}

static void matrix_identity(D3DMATRIX* m) {
  unsigned int r, c;
  for (r = 0; r < 4; ++r) for (c = 0; c < 4; ++c) m->m[r][c] = (r == c) ? 1.0f : 0.0f;
}

static void matrix_ortho_screen(D3DMATRIX* m, float width, float height) {
  float zn = 0.0f;
  float zf = 1.0f;
  float l = 0.0f;
  float r = width > 1.0f ? width : 1.0f;
  float t = 0.0f;
  float b = height > 1.0f ? height : 1.0f;
  matrix_identity(m);
  m->_11 = 2.0f / (r - l);
  m->_22 = 2.0f / (t - b);
  m->_33 = 1.0f / (zf - zn);
  m->_41 = (l + r) / (l - r);
  m->_42 = (t + b) / (b - t);
  m->_43 = zn / (zn - zf);
  m->_44 = 1.0f;
}

static void update_device_size_from_pp(DeviceProxy* self, const D3DPRESENT_PARAMETERS* pp) {
  if (!self || !pp) return;
  if (pp->BackBufferWidth) self->bb_width = pp->BackBufferWidth;
  if (pp->BackBufferHeight) self->bb_height = pp->BackBufferHeight;
}

static void update_device_viewport(DeviceProxy* self, const D3DVIEWPORT9* vp) {
  if (!self || !vp) return;
  self->viewport.X = vp->X;
  self->viewport.Y = vp->Y;
  self->viewport.Width = vp->Width;
  self->viewport.Height = vp->Height;
  self->viewport.MinZ = vp->MinZ;
  self->viewport.MaxZ = vp->MaxZ;
  if (vp->Width) self->bb_width = vp->Width;
  if (vp->Height) self->bb_height = vp->Height;
}

static void copy_presentation_parameters(D3DPRESENT_PARAMETERS* dst, const D3DPRESENT_PARAMETERS* src) {
  if (!dst || !src) return;
  dst->BackBufferWidth = src->BackBufferWidth;
  dst->BackBufferHeight = src->BackBufferHeight;
  dst->BackBufferFormat = src->BackBufferFormat;
  dst->BackBufferCount = src->BackBufferCount;
  dst->MultiSampleType = src->MultiSampleType;
  dst->MultiSampleQuality = src->MultiSampleQuality;
  dst->SwapEffect = src->SwapEffect;
  dst->hDeviceWindow = src->hDeviceWindow;
  dst->Windowed = src->Windowed;
  dst->EnableAutoDepthStencil = src->EnableAutoDepthStencil;
  dst->AutoDepthStencilFormat = src->AutoDepthStencilFormat;
  dst->Flags = src->Flags;
  dst->FullScreen_RefreshRateInHz = src->FullScreen_RefreshRateInHz;
  dst->PresentationInterval = src->PresentationInterval;
}

static void make_windowed_presentation(D3DPRESENT_PARAMETERS* pp, HWND fallback_window) {
  if (!pp) return;
  pp->Windowed = TRUE;
  pp->FullScreen_RefreshRateInHz = 0;
  if (!pp->hDeviceWindow && fallback_window) pp->hDeviceWindow = fallback_window;
}

static int should_force_windowed_for_remix_create(const D3D9Proxy* self) {
  return self && self->is_remix_backend && is_multiverse_game_process() && poptbm_force_windowed_for_remix();
}

static int ensure_rhw_decl(DeviceProxy* self, DWORD fvf) {
  D3DVERTEXELEMENT9 elems[16];
  IDirect3DVertexDeclaration9* decl = NULL;
  HRESULT hr;
  if (!self || !self->real) return 0;
  if (self->rhw_decl && self->rhw_decl_fvf == fvf) return 1;
  if (self->rhw_decl) {
    call_release(self->rhw_decl);
    self->rhw_decl = NULL;
    self->rhw_decl_fvf = 0;
  }
  if (!build_rhw_vertex_declaration(fvf, elems, 16)) return 0;
  hr = call_create_vertex_declaration(self->real, elems, &decl);
  if (FAILED(hr) || !decl) return 0;
  self->rhw_decl = decl;
  self->rhw_decl_fvf = fvf;
  return 1;
}

static void apply_rhw_camera(DeviceProxy* self) {
  D3DMATRIX world, view, proj;
  float w, h;
  if (!self || !self->real) return;
  w = (float)(self->viewport.Width ? self->viewport.Width : (self->bb_width ? self->bb_width : 800));
  h = (float)(self->viewport.Height ? self->viewport.Height : (self->bb_height ? self->bb_height : 600));
  matrix_identity(&world);
  matrix_identity(&view);
  matrix_ortho_screen(&proj, w, h);
  call_set_transform(self->real, D3DTS_WORLD, &world);
  call_set_transform(self->real, D3DTS_VIEW, &view);
  call_set_transform(self->real, D3DTS_PROJECTION, &proj);
}

static void apply_rhw_state_for_draw(DeviceProxy* self) {
  if (!self || !self->rhw_active || !self->game_fvf) return;
  if (!ensure_rhw_decl(self, self->game_fvf)) return;
  apply_rhw_camera(self);
  call_set_vertex_declaration(self->real, self->rhw_decl);
}

static D3D9Proxy* wrap_d3d9_object(void* real, int is_ex, int is_remix_backend) {
  unsigned int i;
  D3D9Proxy* wrapped = NULL;
  if (!real) return NULL;

  lock_proxy_state();
  for (i = 0; i < MAX_D3D9_PROXIES; ++i) {
    if (!g_d3d9_proxy_pool[i].real) {
      g_d3d9_proxy_pool[i].lpVtbl = g_d3d9_vtbl;
      g_d3d9_proxy_pool[i].real = real;
      g_d3d9_proxy_pool[i].magic = PROXY_MAGIC_D3D9;
      g_d3d9_proxy_pool[i].is_ex = is_ex;
      g_d3d9_proxy_pool[i].is_remix_backend = is_remix_backend;
      g_d3d9_proxy_pool[i].sdk_version = 0;
      g_d3d9_proxy_pool[i].create_call_index = g_last_selected_create_call;
      wrapped = &g_d3d9_proxy_pool[i];
      break;
    }
  }
  unlock_proxy_state();
  return wrapped;
}

static DeviceProxy* wrap_device_object(void* real, D3D9Proxy* parent, const D3DPRESENT_PARAMETERS* pp, int is_ex) {
  unsigned int i;
  DeviceProxy* wrapped = NULL;
  if (!real) return NULL;

  lock_proxy_state();
  for (i = 0; i < MAX_DEVICE_PROXIES; ++i) {
    if (!g_device_proxy_pool[i].real) {
      DeviceProxy* d = &g_device_proxy_pool[i];
      d->lpVtbl = g_device_vtbl;
      d->real = real;
      d->magic = PROXY_MAGIC_DEV;
      d->parent = parent;
      d->is_ex = is_ex;
      d->game_fvf = 0;
      d->active_fvf = 0;
      d->rhw_active = 0;
      d->using_game_decl = 0;
      d->vertex_shader_active = 0;
      d->bb_width = 800;
      d->bb_height = 600;
      d->viewport.X = 0;
      d->viewport.Y = 0;
      d->viewport.Width = 800;
      d->viewport.Height = 600;
      d->viewport.MinZ = 0.0f;
      d->viewport.MaxZ = 1.0f;
      d->rhw_decl = NULL;
      d->rhw_decl_fvf = 0;
      d->create_call_index = parent ? parent->create_call_index : g_last_selected_create_call;
      update_device_size_from_pp(d, pp);
      d->viewport.Width = d->bb_width;
      d->viewport.Height = d->bb_height;
      wrapped = d;
      break;
    }
  }
  unlock_proxy_state();
  return wrapped;
}


static int promote_system_proxy_to_remix_for_device(D3D9Proxy* self, const D3DPRESENT_PARAMETERS* pp) {
  HMODULE remix;
  FARPROC proc;
  void* remix_real = NULL;
  HRESULT hr;
  PFN_Direct3DCreate9_Real fn9;
  PFN_Direct3DCreate9Ex_Real fn9ex;

  if (!self || self->magic != PROXY_MAGIC_D3D9 || self->is_remix_backend) return 0;
  if (!is_multiverse_game_process()) return 0;
  if (!poptbm_promote_system_device_with_auto_depth()) return 0;
  if (!pp || !pp->EnableAutoDepthStencil) return 0;

  remix = ensure_remix_d3d9();
  if (!remix) {
    log_backend_promotion(self->create_call_index, "autoDepth", 0);
    return 0;
  }

  if (self->is_ex) {
    proc = get_proc_by_name_or_ordinal(remix, "Direct3DCreate9Ex", 38);
    if (!proc) proc = get_proc_by_name_or_ordinal(remix, "Direct3DCreate9Ex", 14);
    if (!proc) {
      log_backend_promotion(self->create_call_index, "autoDepth", 0);
      return 0;
    }
    fn9ex = (PFN_Direct3DCreate9Ex_Real)proc;
    hr = fn9ex(self->sdk_version, &remix_real);
    if (FAILED(hr) || !remix_real) {
      log_backend_promotion(self->create_call_index, "autoDepth", 0);
      return 0;
    }
  } else {
    proc = get_proc_by_name_or_ordinal(remix, "Direct3DCreate9", 37);
    if (!proc) proc = get_proc_by_name_or_ordinal(remix, "Direct3DCreate9", 13);
    if (!proc) {
      log_backend_promotion(self->create_call_index, "autoDepth", 0);
      return 0;
    }
    fn9 = (PFN_Direct3DCreate9_Real)proc;
    remix_real = fn9(self->sdk_version);
    if (!remix_real) {
      log_backend_promotion(self->create_call_index, "autoDepth", 0);
      return 0;
    }
  }

  /* Keep the old system IDirect3D9 object alive rather than risking refcount
   * mismatch if the game queried extra references before the promotion. It has
   * no device ownership once CreateDevice is routed to Remix, so this is a
   * small bounded leak in exchange for not crossing back into the bridge during
   * menu/game teardown.
   */
  self->real = remix_real;
  self->is_remix_backend = 1;
  lock_proxy_state();
  g_remix_active = 1;
  g_real_d3d9 = remix;
  unlock_proxy_state();
  log_backend_promotion(self->create_call_index, "autoDepth", 1);
  return 1;
}

__attribute__((naked)) void D3D9Forward_3(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *12(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_4(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *16(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_5(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *20(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_9(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *36(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_10(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *40(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_11(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *44(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_12(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *48(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_13(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *52(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_14(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *56(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_15(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *60(%edx)\n"); }
__attribute__((naked)) void D3D9Forward_21(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *84(%edx)\n"); }
__attribute__((naked)) void DeviceForward_3(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *12(%edx)\n"); }
__attribute__((naked)) void DeviceForward_4(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *16(%edx)\n"); }
__attribute__((naked)) void DeviceForward_5(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *20(%edx)\n"); }
__attribute__((naked)) void DeviceForward_7(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *28(%edx)\n"); }
__attribute__((naked)) void DeviceForward_8(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *32(%edx)\n"); }
__attribute__((naked)) void DeviceForward_9(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *36(%edx)\n"); }
__attribute__((naked)) void DeviceForward_10(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *40(%edx)\n"); }
__attribute__((naked)) void DeviceForward_11(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *44(%edx)\n"); }
__attribute__((naked)) void DeviceForward_12(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *48(%edx)\n"); }
__attribute__((naked)) void DeviceForward_13(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *52(%edx)\n"); }
__attribute__((naked)) void DeviceForward_14(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *56(%edx)\n"); }
__attribute__((naked)) void DeviceForward_15(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *60(%edx)\n"); }
__attribute__((naked)) void DeviceForward_17(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *68(%edx)\n"); }
__attribute__((naked)) void DeviceForward_18(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *72(%edx)\n"); }
__attribute__((naked)) void DeviceForward_19(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *76(%edx)\n"); }
__attribute__((naked)) void DeviceForward_20(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *80(%edx)\n"); }
__attribute__((naked)) void DeviceForward_21(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *84(%edx)\n"); }
__attribute__((naked)) void DeviceForward_22(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *88(%edx)\n"); }
__attribute__((naked)) void DeviceForward_23(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *92(%edx)\n"); }
__attribute__((naked)) void DeviceForward_24(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *96(%edx)\n"); }
__attribute__((naked)) void DeviceForward_25(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *100(%edx)\n"); }
__attribute__((naked)) void DeviceForward_26(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *104(%edx)\n"); }
__attribute__((naked)) void DeviceForward_27(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *108(%edx)\n"); }
__attribute__((naked)) void DeviceForward_28(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *112(%edx)\n"); }
__attribute__((naked)) void DeviceForward_29(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *116(%edx)\n"); }
__attribute__((naked)) void DeviceForward_30(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *120(%edx)\n"); }
__attribute__((naked)) void DeviceForward_31(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *124(%edx)\n"); }
__attribute__((naked)) void DeviceForward_32(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *128(%edx)\n"); }
__attribute__((naked)) void DeviceForward_33(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *132(%edx)\n"); }
__attribute__((naked)) void DeviceForward_34(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *136(%edx)\n"); }
__attribute__((naked)) void DeviceForward_35(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *140(%edx)\n"); }
__attribute__((naked)) void DeviceForward_36(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *144(%edx)\n"); }
__attribute__((naked)) void DeviceForward_37(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *148(%edx)\n"); }
__attribute__((naked)) void DeviceForward_38(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *152(%edx)\n"); }
__attribute__((naked)) void DeviceForward_39(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *156(%edx)\n"); }
__attribute__((naked)) void DeviceForward_40(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *160(%edx)\n"); }
__attribute__((naked)) void DeviceForward_41(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *164(%edx)\n"); }
__attribute__((naked)) void DeviceForward_42(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *168(%edx)\n"); }
__attribute__((naked)) void DeviceForward_43(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *172(%edx)\n"); }
__attribute__((naked)) void DeviceForward_45(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *180(%edx)\n"); }
__attribute__((naked)) void DeviceForward_46(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *184(%edx)\n"); }
__attribute__((naked)) void DeviceForward_48(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *192(%edx)\n"); }
__attribute__((naked)) void DeviceForward_49(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *196(%edx)\n"); }
__attribute__((naked)) void DeviceForward_50(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *200(%edx)\n"); }
__attribute__((naked)) void DeviceForward_51(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *204(%edx)\n"); }
__attribute__((naked)) void DeviceForward_52(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *208(%edx)\n"); }
__attribute__((naked)) void DeviceForward_53(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *212(%edx)\n"); }
__attribute__((naked)) void DeviceForward_54(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *216(%edx)\n"); }
__attribute__((naked)) void DeviceForward_55(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *220(%edx)\n"); }
__attribute__((naked)) void DeviceForward_56(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *224(%edx)\n"); }
__attribute__((naked)) void DeviceForward_57(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *228(%edx)\n"); }
__attribute__((naked)) void DeviceForward_58(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *232(%edx)\n"); }
__attribute__((naked)) void DeviceForward_59(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *236(%edx)\n"); }
__attribute__((naked)) void DeviceForward_60(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *240(%edx)\n"); }
__attribute__((naked)) void DeviceForward_61(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *244(%edx)\n"); }
__attribute__((naked)) void DeviceForward_62(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *248(%edx)\n"); }
__attribute__((naked)) void DeviceForward_63(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *252(%edx)\n"); }
__attribute__((naked)) void DeviceForward_64(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *256(%edx)\n"); }
__attribute__((naked)) void DeviceForward_65(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *260(%edx)\n"); }
__attribute__((naked)) void DeviceForward_66(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *264(%edx)\n"); }
__attribute__((naked)) void DeviceForward_67(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *268(%edx)\n"); }
__attribute__((naked)) void DeviceForward_68(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *272(%edx)\n"); }
__attribute__((naked)) void DeviceForward_69(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *276(%edx)\n"); }
__attribute__((naked)) void DeviceForward_70(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *280(%edx)\n"); }
__attribute__((naked)) void DeviceForward_71(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *284(%edx)\n"); }
__attribute__((naked)) void DeviceForward_72(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *288(%edx)\n"); }
__attribute__((naked)) void DeviceForward_73(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *292(%edx)\n"); }
__attribute__((naked)) void DeviceForward_74(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *296(%edx)\n"); }
__attribute__((naked)) void DeviceForward_75(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *300(%edx)\n"); }
__attribute__((naked)) void DeviceForward_76(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *304(%edx)\n"); }
__attribute__((naked)) void DeviceForward_77(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *308(%edx)\n"); }
__attribute__((naked)) void DeviceForward_78(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *312(%edx)\n"); }
__attribute__((naked)) void DeviceForward_79(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *316(%edx)\n"); }
__attribute__((naked)) void DeviceForward_80(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *320(%edx)\n"); }
__attribute__((naked)) void DeviceForward_85(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *340(%edx)\n"); }
__attribute__((naked)) void DeviceForward_88(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *352(%edx)\n"); }
__attribute__((naked)) void DeviceForward_91(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *364(%edx)\n"); }
__attribute__((naked)) void DeviceForward_93(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *372(%edx)\n"); }
__attribute__((naked)) void DeviceForward_94(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *376(%edx)\n"); }
__attribute__((naked)) void DeviceForward_95(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *380(%edx)\n"); }
__attribute__((naked)) void DeviceForward_96(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *384(%edx)\n"); }
__attribute__((naked)) void DeviceForward_97(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *388(%edx)\n"); }
__attribute__((naked)) void DeviceForward_98(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *392(%edx)\n"); }
__attribute__((naked)) void DeviceForward_99(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *396(%edx)\n"); }
__attribute__((naked)) void DeviceForward_101(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *404(%edx)\n"); }
__attribute__((naked)) void DeviceForward_102(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *408(%edx)\n"); }
__attribute__((naked)) void DeviceForward_103(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *412(%edx)\n"); }
__attribute__((naked)) void DeviceForward_104(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *416(%edx)\n"); }
__attribute__((naked)) void DeviceForward_105(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *420(%edx)\n"); }
__attribute__((naked)) void DeviceForward_106(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *424(%edx)\n"); }
__attribute__((naked)) void DeviceForward_107(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *428(%edx)\n"); }
__attribute__((naked)) void DeviceForward_108(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *432(%edx)\n"); }
__attribute__((naked)) void DeviceForward_109(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *436(%edx)\n"); }
__attribute__((naked)) void DeviceForward_110(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *440(%edx)\n"); }
__attribute__((naked)) void DeviceForward_111(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *444(%edx)\n"); }
__attribute__((naked)) void DeviceForward_112(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *448(%edx)\n"); }
__attribute__((naked)) void DeviceForward_113(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *452(%edx)\n"); }
__attribute__((naked)) void DeviceForward_114(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *456(%edx)\n"); }
__attribute__((naked)) void DeviceForward_115(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *460(%edx)\n"); }
__attribute__((naked)) void DeviceForward_116(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *464(%edx)\n"); }
__attribute__((naked)) void DeviceForward_117(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *468(%edx)\n"); }
__attribute__((naked)) void DeviceForward_118(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *472(%edx)\n"); }
__attribute__((naked)) void DeviceForward_119(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *476(%edx)\n"); }
__attribute__((naked)) void DeviceForward_120(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *480(%edx)\n"); }
__attribute__((naked)) void DeviceForward_121(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *484(%edx)\n"); }
__attribute__((naked)) void DeviceForward_122(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *488(%edx)\n"); }
__attribute__((naked)) void DeviceForward_123(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *492(%edx)\n"); }
__attribute__((naked)) void DeviceForward_124(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *496(%edx)\n"); }
__attribute__((naked)) void DeviceForward_125(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *500(%edx)\n"); }
__attribute__((naked)) void DeviceForward_126(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *504(%edx)\n"); }
__attribute__((naked)) void DeviceForward_127(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *508(%edx)\n"); }
__attribute__((naked)) void DeviceForward_128(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *512(%edx)\n"); }
__attribute__((naked)) void DeviceForward_129(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *516(%edx)\n"); }
__attribute__((naked)) void DeviceForward_130(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *520(%edx)\n"); }
__attribute__((naked)) void DeviceForward_131(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *524(%edx)\n"); }
__attribute__((naked)) void DeviceForward_132(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *528(%edx)\n"); }
__attribute__((naked)) void DeviceForward_133(void) { __asm__("movl 4(%esp), %eax\n\t" "movl 4(%eax), %ecx\n\t" "movl %ecx, 4(%esp)\n\t" "movl (%ecx), %edx\n\t" "jmp *532(%edx)\n"); }


static HRESULT __attribute__((stdcall)) D3D9_QueryInterface(D3D9Proxy* self, const GUID* riid, void** ppv) {
  if (!ppv) return E_POINTER;
  if (self && self->magic == PROXY_MAGIC_D3D9 &&
      (guid_eq(riid, &IID_IUnknown_local) || guid_eq(riid, &IID_IDirect3D9_local) || (self->is_ex && guid_eq(riid, &IID_IDirect3D9Ex_local)))) {
    *ppv = self;
    call_addref(self->real);
    return S_OK;
  }
  {
    void** vt = self && self->real ? *(void***)self->real : NULL;
    HRESULT (__attribute__((stdcall)) *fn)(void*, const GUID*, void**) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,const GUID*,void**))vt[0] : NULL;
    return fn ? fn(self->real, riid, ppv) : E_NOINTERFACE;
  }
}

static ULONG __attribute__((stdcall)) D3D9_AddRef(D3D9Proxy* self) {
  return self ? call_addref(self->real) : 1;
}

static ULONG __attribute__((stdcall)) D3D9_Release(D3D9Proxy* self) {
  ULONG refs;
  if (!self) return 0;

  refs = call_release(self->real);
  if (refs == 0) {
    lock_proxy_state();
    self->real = NULL;
    self->lpVtbl = NULL;
    self->magic = 0;
    self->is_ex = 0;
    self->is_remix_backend = 0;
    self->sdk_version = 0;
    self->create_call_index = 0;
    unlock_proxy_state();
  }
  return refs;
}

static UINT __attribute__((stdcall)) D3D9_GetAdapterModeCount(D3D9Proxy* self, UINT Adapter, D3DFORMAT Format) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  UINT (__attribute__((stdcall)) *fn)(void*,UINT,D3DFORMAT) = vt ? (UINT (__attribute__((stdcall)) *)(void*,UINT,D3DFORMAT))vt[6] : NULL;
  return fn ? fn(self->real, Adapter, Format) : 0;
}

static HRESULT __attribute__((stdcall)) D3D9_EnumAdapterModes(D3D9Proxy* self, UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,D3DFORMAT,UINT,D3DDISPLAYMODE*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,D3DFORMAT,UINT,D3DDISPLAYMODE*))vt[7] : NULL;
  return fn ? fn(self->real, Adapter, Format, Mode, pMode) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) D3D9_GetAdapterDisplayMode(D3D9Proxy* self, UINT Adapter, D3DDISPLAYMODE* pMode) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,D3DDISPLAYMODE*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,D3DDISPLAYMODE*))vt[8] : NULL;
  return fn ? fn(self->real, Adapter, pMode) : E_POINTER;
}

static UINT __attribute__((stdcall)) D3D9_GetAdapterModeCountEx(D3D9Proxy* self, UINT Adapter, const void* pFilter) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  UINT (__attribute__((stdcall)) *fn)(void*,UINT,const void*) = vt ? (UINT (__attribute__((stdcall)) *)(void*,UINT,const void*))vt[17] : NULL;
  return fn ? fn(self->real, Adapter, pFilter) : 0;
}

static HRESULT __attribute__((stdcall)) D3D9_EnumAdapterModesEx(D3D9Proxy* self, UINT Adapter, const void* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,const void*,UINT,D3DDISPLAYMODEEX*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,const void*,UINT,D3DDISPLAYMODEEX*))vt[18] : NULL;
  return fn ? fn(self->real, Adapter, pFilter, Mode, pMode) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) D3D9_GetAdapterDisplayModeEx(D3D9Proxy* self, UINT Adapter, D3DDISPLAYMODEEX* pMode, void* pRotation) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,D3DDISPLAYMODEEX*,void*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,D3DDISPLAYMODEEX*,void*))vt[19] : NULL;
  return fn ? fn(self->real, Adapter, pMode, pRotation) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) D3D9_CreateDevice(D3D9Proxy* self, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
  void* real_dev = NULL;
  void** vt;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,void**);
  HRESULT hr;
  DeviceProxy* wrapped;
  D3DPRESENT_PARAMETERS forced_pp;
  D3DPRESENT_PARAMETERS* pp_to_use = pPresentationParameters;
  UINT forced = 0;
  if (!ppReturnedDeviceInterface) return E_POINTER;
  promote_system_proxy_to_remix_for_device(self, pPresentationParameters);
  vt = self && self->real ? *(void***)self->real : NULL;
  fn = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,void**))vt[16] : NULL;
  if (!fn) return E_POINTER;

  if (pPresentationParameters && should_force_windowed_for_remix_create(self)) {
    copy_presentation_parameters(&forced_pp, pPresentationParameters);
    make_windowed_presentation(&forced_pp, hFocusWindow);
    pp_to_use = &forced_pp;
    forced = 1;
  }

  hr = fn(self->real, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pp_to_use, &real_dev);
  log_device_create("CreateDevice", self && self->is_remix_backend ? "remix" : "system", self ? self->create_call_index : 0, pp_to_use, BehaviorFlags, hr, forced);
  if (SUCCEEDED(hr) && real_dev && pp_to_use != pPresentationParameters) {
    copy_presentation_parameters(pPresentationParameters, pp_to_use);
  }
  if (FAILED(hr) || !real_dev || !self || !self->is_remix_backend || !poptbm_enable_rhw_fixup()) {
    *ppReturnedDeviceInterface = (IDirect3DDevice9*)real_dev;
    return hr;
  }
  wrapped = wrap_device_object(real_dev, self, pp_to_use, 0);
  *ppReturnedDeviceInterface = wrapped ? (IDirect3DDevice9*)wrapped : (IDirect3DDevice9*)real_dev;
  return hr;
}

static HRESULT __attribute__((stdcall)) D3D9_CreateDeviceEx(D3D9Proxy* self, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9** ppReturnedDeviceInterface) {
  void* real_dev = NULL;
  void** vt;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,D3DDISPLAYMODEEX*,void**);
  HRESULT hr;
  DeviceProxy* wrapped;
  D3DPRESENT_PARAMETERS forced_pp;
  D3DPRESENT_PARAMETERS* pp_to_use = pPresentationParameters;
  D3DDISPLAYMODEEX* mode_to_use = pFullscreenDisplayMode;
  UINT forced = 0;
  if (!ppReturnedDeviceInterface) return E_POINTER;
  promote_system_proxy_to_remix_for_device(self, pPresentationParameters);
  vt = self && self->real ? *(void***)self->real : NULL;
  fn = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,D3DDISPLAYMODEEX*,void**))vt[20] : NULL;
  if (!fn) return E_POINTER;

  if (pPresentationParameters && should_force_windowed_for_remix_create(self)) {
    copy_presentation_parameters(&forced_pp, pPresentationParameters);
    make_windowed_presentation(&forced_pp, hFocusWindow);
    pp_to_use = &forced_pp;
    mode_to_use = NULL;
    forced = 1;
  }

  hr = fn(self->real, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pp_to_use, mode_to_use, &real_dev);
  log_device_create("CreateDeviceEx", self && self->is_remix_backend ? "remix" : "system", self ? self->create_call_index : 0, pp_to_use, BehaviorFlags, hr, forced);
  if (SUCCEEDED(hr) && real_dev && pp_to_use != pPresentationParameters) {
    copy_presentation_parameters(pPresentationParameters, pp_to_use);
  }
  if (FAILED(hr) || !real_dev || !self || !self->is_remix_backend || !poptbm_enable_rhw_fixup()) {
    *ppReturnedDeviceInterface = (IDirect3DDevice9*)real_dev;
    return hr;
  }
  wrapped = wrap_device_object(real_dev, self, pp_to_use, 1);
  *ppReturnedDeviceInterface = wrapped ? (IDirect3DDevice9*)wrapped : (IDirect3DDevice9*)real_dev;
  return hr;
}

static HRESULT __attribute__((stdcall)) Device_QueryInterface(DeviceProxy* self, const GUID* riid, void** ppv) {
  if (!ppv) return E_POINTER;
  if (self && self->magic == PROXY_MAGIC_DEV &&
      (guid_eq(riid, &IID_IUnknown_local) || guid_eq(riid, &IID_IDirect3DDevice9_local) || (self->is_ex && guid_eq(riid, &IID_IDirect3DDevice9Ex_local)))) {
    *ppv = self;
    call_addref(self->real);
    return S_OK;
  }
  {
    void** vt = self && self->real ? *(void***)self->real : NULL;
    HRESULT (__attribute__((stdcall)) *fn)(void*, const GUID*, void**) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,const GUID*,void**))vt[0] : NULL;
    return fn ? fn(self->real, riid, ppv) : E_NOINTERFACE;
  }
}

static ULONG __attribute__((stdcall)) Device_AddRef(DeviceProxy* self) {
  return self ? call_addref(self->real) : 1;
}

static ULONG __attribute__((stdcall)) Device_Release(DeviceProxy* self) {
  ULONG refs_before;
  ULONG refs;
  if (!self || !self->real) return 0;

  /*
   * Release selector-owned device children before the underlying device reaches
   * refcount zero. Releasing the synthetic vertex declaration after the bridge
   * has already destroyed the device can enqueue late resource-destroy commands
   * into the 64-bit server while it is tearing the D3D9 module down.
   */
  refs_before = call_addref(self->real);
  refs_before = call_release(self->real);
  if (refs_before <= 1 && self->rhw_decl) {
    call_set_vertex_declaration(self->real, NULL);
    call_release(self->rhw_decl);
    self->rhw_decl = NULL;
    self->rhw_decl_fvf = 0;
  }

  refs = call_release(self->real);
  if (refs == 0) {
    lock_proxy_state();
    self->real = NULL;
    self->lpVtbl = NULL;
    self->magic = 0;
    self->parent = NULL;
    self->is_ex = 0;
    self->game_fvf = 0;
    self->active_fvf = 0;
    self->rhw_active = 0;
    self->using_game_decl = 0;
    self->vertex_shader_active = 0;
    self->rhw_decl = NULL;
    self->rhw_decl_fvf = 0;
    self->create_call_index = 0;
    unlock_proxy_state();
  }
  return refs;
}

static HRESULT __attribute__((stdcall)) Device_GetDirect3D(DeviceProxy* self, IDirect3D9** ppD3D9) {
  if (!ppD3D9) return E_POINTER;
  if (self && self->parent) {
    *ppD3D9 = (IDirect3D9*)self->parent;
    call_addref(self->parent->real);
    return S_OK;
  }
  return E_POINTER;
}

static HRESULT __attribute__((stdcall)) Device_Reset(DeviceProxy* self, D3DPRESENT_PARAMETERS* pp) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,D3DPRESENT_PARAMETERS*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,D3DPRESENT_PARAMETERS*))vt[16] : NULL;
  HRESULT hr = fn ? fn(self->real, pp) : E_POINTER;
  if (SUCCEEDED(hr)) update_device_size_from_pp(self, pp);
  return hr;
}

static HRESULT __attribute__((stdcall)) Device_SetTransform(DeviceProxy* self, DWORD State, const D3DMATRIX* pMatrix) {
  if (self && self->rhw_active && (State == D3DTS_WORLD || State == D3DTS_VIEW || State == D3DTS_PROJECTION)) {
    /* RHW draws need the synthetic camera to survive wrapper/game matrix noise. */
    return S_OK;
  }
  return call_set_transform(self ? self->real : NULL, State, pMatrix);
}

static HRESULT __attribute__((stdcall)) Device_SetViewport(DeviceProxy* self, const D3DVIEWPORT9* pViewport) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,const D3DVIEWPORT9*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,const D3DVIEWPORT9*))vt[47] : NULL;
  HRESULT hr = fn ? fn(self->real, pViewport) : E_POINTER;
  if (SUCCEEDED(hr)) update_device_viewport(self, pViewport);
  return hr;
}

static HRESULT __attribute__((stdcall)) Device_SetFVF(DeviceProxy* self, DWORD fvf) {
  if (!self) return E_POINTER;
  self->game_fvf = fvf;
  self->active_fvf = fvf;
  self->using_game_decl = 0;
  if ((fvf & D3DFVF_XYZRHW) && ensure_rhw_decl(self, fvf)) {
    self->rhw_active = 1;
    apply_rhw_camera(self);
    return call_set_vertex_declaration(self->real, self->rhw_decl);
  }
  self->rhw_active = 0;
  return call_set_fvf(self->real, fvf);
}

static HRESULT __attribute__((stdcall)) Device_GetFVF(DeviceProxy* self, DWORD* pFVF) {
  if (!pFVF) return E_POINTER;
  if (self && self->game_fvf) {
    *pFVF = self->game_fvf;
    return S_OK;
  }
  {
    void** vt = self && self->real ? *(void***)self->real : NULL;
    HRESULT (__attribute__((stdcall)) *fn)(void*,DWORD*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,DWORD*))vt[90] : NULL;
    return fn ? fn(self->real, pFVF) : E_POINTER;
  }
}

static HRESULT __attribute__((stdcall)) Device_CreateVertexDeclaration(DeviceProxy* self, const D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
  return call_create_vertex_declaration(self ? self->real : NULL, pVertexElements, ppDecl);
}

static HRESULT __attribute__((stdcall)) Device_SetVertexDeclaration(DeviceProxy* self, IDirect3DVertexDeclaration9* pDecl) {
  if (!self) return E_POINTER;
  self->using_game_decl = 1;
  self->rhw_active = 0;
  return call_set_vertex_declaration(self->real, pDecl);
}

static HRESULT __attribute__((stdcall)) Device_SetVertexShader(DeviceProxy* self, void* shader) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,void*) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,void*))vt[92] : NULL;
  if (self) self->vertex_shader_active = shader ? 1 : 0;
  return fn ? fn(self->real, shader) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) Device_SetStreamSource(DeviceProxy* self, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,IDirect3DVertexBuffer9*,UINT,UINT) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,IDirect3DVertexBuffer9*,UINT,UINT))vt[100] : NULL;
  return fn ? fn(self->real, StreamNumber, pStreamData, OffsetInBytes, Stride) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) Device_DrawPrimitive(DeviceProxy* self, UINT PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,UINT,UINT) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,UINT,UINT))vt[81] : NULL;
  if (self && self->rhw_active) apply_rhw_state_for_draw(self);
  return fn ? fn(self->real, PrimitiveType, StartVertex, PrimitiveCount) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) Device_DrawIndexedPrimitive(DeviceProxy* self, UINT PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,INT,UINT,UINT,UINT,UINT) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,INT,UINT,UINT,UINT,UINT))vt[82] : NULL;
  if (self && self->rhw_active) apply_rhw_state_for_draw(self);
  return fn ? fn(self->real, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) Device_DrawPrimitiveUP(DeviceProxy* self, UINT PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,UINT,const void*,UINT) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,UINT,const void*,UINT))vt[83] : NULL;
  if (self && self->rhw_active) apply_rhw_state_for_draw(self);
  return fn ? fn(self->real, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) : E_POINTER;
}

static HRESULT __attribute__((stdcall)) Device_DrawIndexedPrimitiveUP(DeviceProxy* self, UINT PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void* pIndexData, UINT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
  void** vt = self && self->real ? *(void***)self->real : NULL;
  HRESULT (__attribute__((stdcall)) *fn)(void*,UINT,UINT,UINT,UINT,const void*,UINT,const void*,UINT) = vt ? (HRESULT (__attribute__((stdcall)) *)(void*,UINT,UINT,UINT,UINT,const void*,UINT,const void*,UINT))vt[84] : NULL;
  if (self && self->rhw_active) apply_rhw_state_for_draw(self);
  return fn ? fn(self->real, PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) : E_POINTER;
}

static void init_proxy_vtables(void) {
  if (g_proxy_vtables_init) return;
  g_d3d9_vtbl[0] = (void*)D3D9_QueryInterface;
  g_d3d9_vtbl[1] = (void*)D3D9_AddRef;
  g_d3d9_vtbl[2] = (void*)D3D9_Release;
  g_d3d9_vtbl[3] = (void*)D3D9Forward_3;
  g_d3d9_vtbl[4] = (void*)D3D9Forward_4;
  g_d3d9_vtbl[5] = (void*)D3D9Forward_5;
  g_d3d9_vtbl[6] = (void*)D3D9_GetAdapterModeCount;
  g_d3d9_vtbl[7] = (void*)D3D9_EnumAdapterModes;
  g_d3d9_vtbl[8] = (void*)D3D9_GetAdapterDisplayMode;
  g_d3d9_vtbl[9] = (void*)D3D9Forward_9;
  g_d3d9_vtbl[10] = (void*)D3D9Forward_10;
  g_d3d9_vtbl[11] = (void*)D3D9Forward_11;
  g_d3d9_vtbl[12] = (void*)D3D9Forward_12;
  g_d3d9_vtbl[13] = (void*)D3D9Forward_13;
  g_d3d9_vtbl[14] = (void*)D3D9Forward_14;
  g_d3d9_vtbl[15] = (void*)D3D9Forward_15;
  g_d3d9_vtbl[16] = (void*)D3D9_CreateDevice;
  g_d3d9_vtbl[17] = (void*)D3D9_GetAdapterModeCountEx;
  g_d3d9_vtbl[18] = (void*)D3D9_EnumAdapterModesEx;
  g_d3d9_vtbl[19] = (void*)D3D9_GetAdapterDisplayModeEx;
  g_d3d9_vtbl[20] = (void*)D3D9_CreateDeviceEx;
  g_d3d9_vtbl[21] = (void*)D3D9Forward_21;
  g_device_vtbl[0] = (void*)Device_QueryInterface;
  g_device_vtbl[1] = (void*)Device_AddRef;
  g_device_vtbl[2] = (void*)Device_Release;
  g_device_vtbl[3] = (void*)DeviceForward_3;
  g_device_vtbl[4] = (void*)DeviceForward_4;
  g_device_vtbl[5] = (void*)DeviceForward_5;
  g_device_vtbl[6] = (void*)Device_GetDirect3D;
  g_device_vtbl[7] = (void*)DeviceForward_7;
  g_device_vtbl[8] = (void*)DeviceForward_8;
  g_device_vtbl[9] = (void*)DeviceForward_9;
  g_device_vtbl[10] = (void*)DeviceForward_10;
  g_device_vtbl[11] = (void*)DeviceForward_11;
  g_device_vtbl[12] = (void*)DeviceForward_12;
  g_device_vtbl[13] = (void*)DeviceForward_13;
  g_device_vtbl[14] = (void*)DeviceForward_14;
  g_device_vtbl[15] = (void*)DeviceForward_15;
  g_device_vtbl[16] = (void*)Device_Reset;
  g_device_vtbl[17] = (void*)DeviceForward_17;
  g_device_vtbl[18] = (void*)DeviceForward_18;
  g_device_vtbl[19] = (void*)DeviceForward_19;
  g_device_vtbl[20] = (void*)DeviceForward_20;
  g_device_vtbl[21] = (void*)DeviceForward_21;
  g_device_vtbl[22] = (void*)DeviceForward_22;
  g_device_vtbl[23] = (void*)DeviceForward_23;
  g_device_vtbl[24] = (void*)DeviceForward_24;
  g_device_vtbl[25] = (void*)DeviceForward_25;
  g_device_vtbl[26] = (void*)DeviceForward_26;
  g_device_vtbl[27] = (void*)DeviceForward_27;
  g_device_vtbl[28] = (void*)DeviceForward_28;
  g_device_vtbl[29] = (void*)DeviceForward_29;
  g_device_vtbl[30] = (void*)DeviceForward_30;
  g_device_vtbl[31] = (void*)DeviceForward_31;
  g_device_vtbl[32] = (void*)DeviceForward_32;
  g_device_vtbl[33] = (void*)DeviceForward_33;
  g_device_vtbl[34] = (void*)DeviceForward_34;
  g_device_vtbl[35] = (void*)DeviceForward_35;
  g_device_vtbl[36] = (void*)DeviceForward_36;
  g_device_vtbl[37] = (void*)DeviceForward_37;
  g_device_vtbl[38] = (void*)DeviceForward_38;
  g_device_vtbl[39] = (void*)DeviceForward_39;
  g_device_vtbl[40] = (void*)DeviceForward_40;
  g_device_vtbl[41] = (void*)DeviceForward_41;
  g_device_vtbl[42] = (void*)DeviceForward_42;
  g_device_vtbl[43] = (void*)DeviceForward_43;
  g_device_vtbl[44] = (void*)Device_SetTransform;
  g_device_vtbl[45] = (void*)DeviceForward_45;
  g_device_vtbl[46] = (void*)DeviceForward_46;
  g_device_vtbl[47] = (void*)Device_SetViewport;
  g_device_vtbl[48] = (void*)DeviceForward_48;
  g_device_vtbl[49] = (void*)DeviceForward_49;
  g_device_vtbl[50] = (void*)DeviceForward_50;
  g_device_vtbl[51] = (void*)DeviceForward_51;
  g_device_vtbl[52] = (void*)DeviceForward_52;
  g_device_vtbl[53] = (void*)DeviceForward_53;
  g_device_vtbl[54] = (void*)DeviceForward_54;
  g_device_vtbl[55] = (void*)DeviceForward_55;
  g_device_vtbl[56] = (void*)DeviceForward_56;
  g_device_vtbl[57] = (void*)DeviceForward_57;
  g_device_vtbl[58] = (void*)DeviceForward_58;
  g_device_vtbl[59] = (void*)DeviceForward_59;
  g_device_vtbl[60] = (void*)DeviceForward_60;
  g_device_vtbl[61] = (void*)DeviceForward_61;
  g_device_vtbl[62] = (void*)DeviceForward_62;
  g_device_vtbl[63] = (void*)DeviceForward_63;
  g_device_vtbl[64] = (void*)DeviceForward_64;
  g_device_vtbl[65] = (void*)DeviceForward_65;
  g_device_vtbl[66] = (void*)DeviceForward_66;
  g_device_vtbl[67] = (void*)DeviceForward_67;
  g_device_vtbl[68] = (void*)DeviceForward_68;
  g_device_vtbl[69] = (void*)DeviceForward_69;
  g_device_vtbl[70] = (void*)DeviceForward_70;
  g_device_vtbl[71] = (void*)DeviceForward_71;
  g_device_vtbl[72] = (void*)DeviceForward_72;
  g_device_vtbl[73] = (void*)DeviceForward_73;
  g_device_vtbl[74] = (void*)DeviceForward_74;
  g_device_vtbl[75] = (void*)DeviceForward_75;
  g_device_vtbl[76] = (void*)DeviceForward_76;
  g_device_vtbl[77] = (void*)DeviceForward_77;
  g_device_vtbl[78] = (void*)DeviceForward_78;
  g_device_vtbl[79] = (void*)DeviceForward_79;
  g_device_vtbl[80] = (void*)DeviceForward_80;
  g_device_vtbl[81] = (void*)Device_DrawPrimitive;
  g_device_vtbl[82] = (void*)Device_DrawIndexedPrimitive;
  g_device_vtbl[83] = (void*)Device_DrawPrimitiveUP;
  g_device_vtbl[84] = (void*)Device_DrawIndexedPrimitiveUP;
  g_device_vtbl[85] = (void*)DeviceForward_85;
  g_device_vtbl[86] = (void*)Device_CreateVertexDeclaration;
  g_device_vtbl[87] = (void*)Device_SetVertexDeclaration;
  g_device_vtbl[88] = (void*)DeviceForward_88;
  g_device_vtbl[89] = (void*)Device_SetFVF;
  g_device_vtbl[90] = (void*)Device_GetFVF;
  g_device_vtbl[91] = (void*)DeviceForward_91;
  g_device_vtbl[92] = (void*)Device_SetVertexShader;
  g_device_vtbl[93] = (void*)DeviceForward_93;
  g_device_vtbl[94] = (void*)DeviceForward_94;
  g_device_vtbl[95] = (void*)DeviceForward_95;
  g_device_vtbl[96] = (void*)DeviceForward_96;
  g_device_vtbl[97] = (void*)DeviceForward_97;
  g_device_vtbl[98] = (void*)DeviceForward_98;
  g_device_vtbl[99] = (void*)DeviceForward_99;
  g_device_vtbl[100] = (void*)Device_SetStreamSource;
  g_device_vtbl[101] = (void*)DeviceForward_101;
  g_device_vtbl[102] = (void*)DeviceForward_102;
  g_device_vtbl[103] = (void*)DeviceForward_103;
  g_device_vtbl[104] = (void*)DeviceForward_104;
  g_device_vtbl[105] = (void*)DeviceForward_105;
  g_device_vtbl[106] = (void*)DeviceForward_106;
  g_device_vtbl[107] = (void*)DeviceForward_107;
  g_device_vtbl[108] = (void*)DeviceForward_108;
  g_device_vtbl[109] = (void*)DeviceForward_109;
  g_device_vtbl[110] = (void*)DeviceForward_110;
  g_device_vtbl[111] = (void*)DeviceForward_111;
  g_device_vtbl[112] = (void*)DeviceForward_112;
  g_device_vtbl[113] = (void*)DeviceForward_113;
  g_device_vtbl[114] = (void*)DeviceForward_114;
  g_device_vtbl[115] = (void*)DeviceForward_115;
  g_device_vtbl[116] = (void*)DeviceForward_116;
  g_device_vtbl[117] = (void*)DeviceForward_117;
  g_device_vtbl[118] = (void*)DeviceForward_118;
  g_device_vtbl[119] = (void*)DeviceForward_119;
  g_device_vtbl[120] = (void*)DeviceForward_120;
  g_device_vtbl[121] = (void*)DeviceForward_121;
  g_device_vtbl[122] = (void*)DeviceForward_122;
  g_device_vtbl[123] = (void*)DeviceForward_123;
  g_device_vtbl[124] = (void*)DeviceForward_124;
  g_device_vtbl[125] = (void*)DeviceForward_125;
  g_device_vtbl[126] = (void*)DeviceForward_126;
  g_device_vtbl[127] = (void*)DeviceForward_127;
  g_device_vtbl[128] = (void*)DeviceForward_128;
  g_device_vtbl[129] = (void*)DeviceForward_129;
  g_device_vtbl[130] = (void*)DeviceForward_130;
  g_device_vtbl[131] = (void*)DeviceForward_131;
  g_device_vtbl[132] = (void*)DeviceForward_132;
  g_device_vtbl[133] = (void*)DeviceForward_133;
  g_proxy_vtables_init = 1;
}

void* __attribute__((stdcall)) Direct3DCreate9_proxy(UINT SDKVersion) {
  HMODULE selected;
  FARPROC proc;
  PFN_Direct3DCreate9_Real fn;
  void* real;
  D3D9Proxy* wrapped;
  init_proxy_vtables();
  selected = select_d3d9_for_create("Direct3DCreate9");
  proc = get_proc_by_name_or_ordinal(selected, "Direct3DCreate9", 37);
  if (!proc) proc = get_proc_by_name_or_ordinal(selected, "Direct3DCreate9", 13);
  if (!proc) {
    HMODULE system = ensure_system_d3d9();
    proc = get_proc_by_name_or_ordinal(system, "Direct3DCreate9", 37);
    if (!proc) proc = get_proc_by_name_or_ordinal(system, "Direct3DCreate9", 13);
  }
  if (!proc) {
    MissingExportFailure_stub();
    return NULL;
  }
  fn = (PFN_Direct3DCreate9_Real)proc;
  real = fn(SDKVersion);
  if (!real || !is_game_process()) return real;
  wrapped = wrap_d3d9_object(real, 0, (selected == g_remix_d3d9 && g_remix_d3d9 != NULL));
  if (wrapped) wrapped->sdk_version = SDKVersion;
  return wrapped ? (void*)wrapped : real;
}

HRESULT __attribute__((stdcall)) Direct3DCreate9Ex_proxy(UINT SDKVersion, void** ppD3D9Ex) {
  HMODULE selected;
  FARPROC proc;
  PFN_Direct3DCreate9Ex_Real fn;
  void* real = NULL;
  HRESULT hr;
  D3D9Proxy* wrapped;
  if (!ppD3D9Ex) return E_POINTER;
  *ppD3D9Ex = NULL;
  init_proxy_vtables();
  selected = select_d3d9_for_create("Direct3DCreate9Ex");
  proc = get_proc_by_name_or_ordinal(selected, "Direct3DCreate9Ex", 38);
  if (!proc) proc = get_proc_by_name_or_ordinal(selected, "Direct3DCreate9Ex", 14);
  if (!proc) {
    HMODULE system = ensure_system_d3d9();
    proc = get_proc_by_name_or_ordinal(system, "Direct3DCreate9Ex", 38);
    if (!proc) proc = get_proc_by_name_or_ordinal(system, "Direct3DCreate9Ex", 14);
  }
  if (!proc) {
    MissingExportFailure_stub();
    return (HRESULT)0x8876086C;
  }
  fn = (PFN_Direct3DCreate9Ex_Real)proc;
  hr = fn(SDKVersion, &real);
  if (FAILED(hr) || !real || !is_game_process()) {
    *ppD3D9Ex = real;
    return hr;
  }
  wrapped = wrap_d3d9_object(real, 1, (selected == g_remix_d3d9 && g_remix_d3d9 != NULL));
  if (wrapped) wrapped->sdk_version = SDKVersion;
  *ppD3D9Ex = wrapped ? (void*)wrapped : real;
  return hr;
}

#define EXPORT_STUB(fn) \
  const char fn##_name[] = #fn; \
  __attribute__((naked)) void fn##_stub(void) { \
    __asm__("pushl $_" #fn "_name\n\t" \
            "call _ResolveExportByName\n\t" \
            "addl $4, %esp\n\t" \
            "testl %eax, %eax\n\t" \
            "jz _MissingExportFailure_stub\n\t" \
            "jmp *%eax\n"); \
  }

#define ORDINAL_STUB(ord) \
  __attribute__((naked)) void Ordinal##ord##_stub(void) { \
    __asm__("pushl $" #ord "\n\t" \
            "call _ResolveExportByOrdinal\n\t" \
            "addl $4, %esp\n\t" \
            "testl %eax, %eax\n\t" \
            "jz _MissingExportFailure_stub\n\t" \
            "jmp *%eax\n"); \
  }

ORDINAL_STUB(16)
ORDINAL_STUB(17)
ORDINAL_STUB(18)
ORDINAL_STUB(19)
ORDINAL_STUB(22)
ORDINAL_STUB(23)

EXPORT_STUB(D3DPERF_BeginEvent)
EXPORT_STUB(D3DPERF_EndEvent)
EXPORT_STUB(D3DPERF_GetStatus)
EXPORT_STUB(D3DPERF_QueryRepeatFrame)
EXPORT_STUB(D3DPERF_SetMarker)
EXPORT_STUB(D3DPERF_SetOptions)
EXPORT_STUB(D3DPERF_SetRegion)
EXPORT_STUB(DebugSetLevel)
EXPORT_STUB(DebugSetMute)
EXPORT_STUB(Direct3D9EnableMaximizedWindowedModeShim)
EXPORT_STUB(Direct3DCreate9On12)
EXPORT_STUB(Direct3DCreate9On12Ex)
EXPORT_STUB(Direct3DShaderValidatorCreate9)
EXPORT_STUB(PSGPError)
EXPORT_STUB(PSGPSampleTexture)

BOOL __attribute__((stdcall)) DllMain(HINSTANCE hinst, DWORD reason, PVOID reserved) {
  (void)reserved;
  if (reason == DLL_PROCESS_ATTACH) {
    g_self_module = hinst;
  }
  return TRUE;
}
