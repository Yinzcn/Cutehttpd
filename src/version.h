
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#define CHTD_VERSION "0.1a"
#define BUILDTIME __DATE__ " " __TIME__
#ifdef _MSC_VER
    #define CPER "MS_VC"
    #define CVER _MSC_VER
#endif
#ifdef __GNUC__
    #define CPER "GNU_C"
    #define CVER (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#endif
#ifdef __TINYC__
    #undef CPER
    #undef CVER
    #define CPER "Tiny_C"
    #define CVER __TINYC__
#endif
#ifdef __POCC__
    #undef CPER
    #undef CVER
    #define CPER "Pelles_C"
    #define CVER __POCC__
#endif
#ifdef __LCC__
    #undef CPER
    #undef CVER
    #define CPER "Lcc"
    #define CVER 0
#endif
#ifndef CPER
    #define CPER "Unknown"
    #define CVER 0
#endif
#ifndef REV_A
    #define REV_A 0
#endif
#ifndef REV_B
    #define REV_B 0
#endif
