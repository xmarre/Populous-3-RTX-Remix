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
typedef const char*        LPCSTR;
typedef WCHAR*             LPWSTR;
typedef const WCHAR*       LPCWSTR;

#define TRUE 1
#define FALSE 0

typedef HMODULE (__attribute__((stdcall)) *PFN_LoadLibraryW)(LPCWSTR);
typedef FARPROC (__attribute__((stdcall)) *PFN_GetProcAddress)(HMODULE, LPCSTR);
typedef DWORD   (__attribute__((stdcall)) *PFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
typedef UINT    (__attribute__((stdcall)) *PFN_GetSystemDirectoryW)(LPWSTR, UINT);

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
static HMODULE g_system_d3d9 = NULL;
static PFN_LoadLibraryW pLoadLibraryW = NULL;
static PFN_GetProcAddress pGetProcAddress = NULL;
static PFN_GetModuleFileNameW pGetModuleFileNameW = NULL;
static PFN_GetSystemDirectoryW pGetSystemDirectoryW = NULL;

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

  if (pLoadLibraryW && pGetProcAddress && pGetModuleFileNameW && pGetSystemDirectoryW) return 1;

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

  return pLoadLibraryW && pGetProcAddress && pGetModuleFileNameW && pGetSystemDirectoryW;
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

static int is_game_process(void) {
  static const WCHAR POPTBM_EXE[]   = { 'p','o','p','T','B','M','.','e','x','e',0 };
  static const WCHAR D3DPOPTB_EXE[] = { 'D','3','D','P','o','p','T','B','.','e','x','e',0 };
  static const WCHAR POPTB_EXE[]    = { 'p','o','p','T','B','.','e','x','e',0 };
  WCHAR exe_path[MAX_PATH_CHARS];
  const WCHAR* base;

  exe_path[0] = 0;
  if (!ensure_kernel_functions()) return 0;
  if (!pGetModuleFileNameW(NULL, exe_path, MAX_PATH_CHARS)) return 0;
  base = basename_from_path(exe_path);

  return wide_basename_eq(base, POPTBM_EXE) ||
         wide_basename_eq(base, D3DPOPTB_EXE) ||
         wide_basename_eq(base, POPTB_EXE);
}

static HMODULE load_system_d3d9(void) {
  static const WCHAR SLASH_D3D9_DLL[] = { '\\','d','3','d','9','.','d','l','l',0 };
  WCHAR path[MAX_PATH_CHARS];
  path[0] = 0;
  if (!ensure_kernel_functions()) return NULL;
  if (!pGetSystemDirectoryW(path, MAX_PATH_CHARS)) return NULL;
  wide_append(path, MAX_PATH_CHARS, SLASH_D3D9_DLL);
  return pLoadLibraryW(path);
}

static HMODULE ensure_system_d3d9(void) {
  HMODULE mod = g_system_d3d9;
  if (mod) return mod;
  mod = load_system_d3d9();
  g_system_d3d9 = mod;
  return mod;
}

static HMODULE load_remix_bridge(void) {
  static const WCHAR REMIX_BRIDGE_NAME[] = { '\\','d','3','d','9','-','r','e','m','i','x','.','d','l','l',0 };
  WCHAR path[MAX_PATH_CHARS];
  path[0] = 0;
  if (!ensure_kernel_functions()) return NULL;
  if (!pGetModuleFileNameW((HMODULE)g_self_module, path, MAX_PATH_CHARS)) return NULL;
  directory_from_path(path);
  wide_append(path, MAX_PATH_CHARS, REMIX_BRIDGE_NAME);
  return pLoadLibraryW(path);
}

static HMODULE ensure_real_d3d9(void) {
  HMODULE mod = g_real_d3d9;
  if (mod) return mod;

  if (is_game_process()) {
    mod = load_remix_bridge();
    if (!mod) mod = ensure_system_d3d9();
  } else {
    mod = ensure_system_d3d9();
  }

  g_real_d3d9 = mod;
  return mod;
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

#define EXPORT_STUB(fn) \
  const char fn##_name[] = #fn; \
  __attribute__((naked)) void fn##_stub(void) { \
    __asm__("pushl $_" #fn "_name\n\t" \
            "call _ResolveExportByName\n\t" \
            "addl $4, %esp\n\t" \
            "jmp *%eax\n"); \
  }

#define ORDINAL_STUB(ord) \
  __attribute__((naked)) void Ordinal##ord##_stub(void) { \
    __asm__("pushl $" #ord "\n\t" \
            "call _ResolveExportByOrdinal\n\t" \
            "addl $4, %esp\n\t" \
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
EXPORT_STUB(Direct3DCreate9)
EXPORT_STUB(Direct3DCreate9Ex)
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
