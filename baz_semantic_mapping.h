// baz_semantic_mapping.h - BASE150000 Unicode mapping table (template, see comments)
// Author: Edouardo-Marcellino Fernandes
// Motto: "By the fog, the impossible dies."
#ifndef BAZ_SEMANTIC_MAPPING_H
#define BAZ_SEMANTIC_MAPPING_H

#define BAZ_BASE 150000
#define BAZ_TABLE_SIZE 150000
#define BAZ_EMPTY 0x000000

// Zone boundary indices
#define BAZ_ZONE_CJK_START      0
#define BAZ_ZONE_LATIN_START    10000
#define BAZ_ZONE_HEALTH         20000
#define BAZ_ZONE_EMOJI_ROUTINE  20100
#define BAZ_ZONE_EMOJI_COMMS    20200
#define BAZ_ZONE_EMOJI_ALERT    20300
#define BAZ_ZONE_EMOJI_COMPUTE  20400
#define BAZ_ZONE_EMOJI_NAV      20500
#define BAZ_ZONE_EMOJI_NATURE   20600
#define BAZ_ZONE_EMOJI_SOCIAL   20700
#define BAZ_ZONE_BALISEE1       30000
#define BAZ_ZONE_DIGITS         50000
#define BAZ_ZONE_BALISEE2       60000
#define BAZ_ZONE_MATHPIC        80000
#define BAZ_ZONE_BALISEE3       90000
#define BAZ_ZONE_PUNCT          100000
#define BAZ_ZONE_BALISEE4       110000

// Macro to check if an index is BALISEE
#define baz_is_balisee(i) (((i) >= 30000 && (i) < 50000) || ((i) >= 60000 && (i) < 80000) || ((i) >= 90000 && (i) < 100000) || ((i) >= 110000 && (i) < 150000))

// Example opcode (zone) defines
#define BAZ_CP_HEALTH_PILL   0x1F48A   // Index 20000
#define BAZ_CP_ROUTINE_SUN   0x2600    // Index 20100
#define BAZ_CP_COMMS_PHONE   0x1F4DE   // Index 20200
#define BAZ_CP_ALERT_FIRE    0x1F525   // Index 20300
#define BAZ_CP_COMPUTE_LAP   0x1F4BB   // Index 20400
#define BAZ_CP_NAV_ROCKET    0x1F680   // Index 20500
#define BAZ_CP_NATURE_SEED   0x1F331   // Index 20600
#define BAZ_CP_SOCIAL_WAVE   0x1F44B   // Index 20700

// Top of digit zone
#define BAZ_CP_DIGIT_0       0x0030    // Index 50000
#define BAZ_CP_DIGIT_1       0x0031
// ... (0-9)

// Example math pictogram
#define BAZ_CP_SUM           0x2211    // Index 80000

// Top of punctuation zone
#define BAZ_CP_COLON         0x003A    // Index 100000
#define BAZ_CP_SLASH         0x002F
#define BAZ_CP_AT            0x0040
#define BAZ_CP_DOT           0x002E
#define BAZ_CP_HASH          0x0023

// Mapping table (first several, zones, demo; rest to be filled by generator)
static const unsigned int baz_int_to_cp[BAZ_TABLE_SIZE] = {
    // 0-9999 CJK UNIFIED IDEOGRAPHS U+4E00+
    0x4E00, 0x4E01, 0x4E02, 0x4E03, 0x4E04, 0x4E05, 0x4E06, 0x4E07, 0x4E08, 0x4E09, // ... up to 0x4E00+9999
    // 10000 Latin/Greek/Cyrillic etc.
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, // a-j
    0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, // k-t
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, // u-z
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, // A-J
    0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, // K-T
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, // U-Z
    0x391, 0x392, 0x393, 0x394, 0x395, // Greek 
    0x3B1, 0x3B2, 0x3B3, 0x3B4, 0x3B5, // greek
    0x410, 0x411, 0x412, 0x413, 0x414, // Cyrillic
    /* ...for demo, only a sample. Expand per zone spec. Others fill with BAZ_EMPTY=0x000000 */
    // 20000+ HEALTH EMOJIS
    0x1F48A, // 💊 PILL
    /* ...similarly, emoji sample... */
    // 20100+ ROUTINE
    0x2600, // ☀ SUN
    // etc.
    // All undefined entries:
    [20701 ... 29999] = BAZ_EMPTY,
    [30000 ... 49999] = BAZ_EMPTY,
    [60000 ... 79999] = BAZ_EMPTY,
    [90000 ... 99999] = BAZ_EMPTY,
    [110000 ... 149999] = BAZ_EMPTY
};

#endif // BAZ_SEMANTIC_MAPPING_H
