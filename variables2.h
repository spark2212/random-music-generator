// ///////////////////////////
// variables.h
// ///////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAJOR 0
#define MINOR 1
#define DIMINISHED 2
#define HALF_DIMINISHED_7 3
#define FULLY_DIMINISHED_7 4
#define DOMINANT_7 5

#define NUM_MODES 2
#define NUM_TONIC_CHORDS 5
#define NUM_SUBDOMINANT_CHORDS 4
#define NUM_DOMINANT_CHORDS 5

#define TONIC 0
#define SUBDOMINANT 1
#define DOMINANT 2
#define UNDEFINED 3
#define NUM_TYPES 3

#define NUM_OPTIONS 5

#define SCALE_SIZE 8
#define SCALE2_SIZE 15

#define WHOLE 4.0
#define DOT_HALF 3.0
#define HALF 2.0
#define DOT_QUARTER 1.5
#define QUARTER 1.0
#define EIGHTH 0.5
#define SIXTEENTH 0.25
#define NUM_LENGTHS 7
#define LENGTHS_SIZE 6

#define RATE 22050.0
#define A4 440.0
#define A3 220.0

#define MAX_TRIES 100000

#define SOPRANO 0
#define ALTO 1
#define TENOR 2
#define BASS 3
#define NUM_VOICES 4

#define THREE 3
#define OCTAVE 12

// Rule Numbers
#define NO_DLT 0
#define LT_RESOLUTION 1
#define NO_MELODIC_TT 2
#define NO_PARALLELS 3
#define MINIMAL_MOVEMENT 4
#define NUM_RULES 5

#define TEMP 0
#define NEW 1

#define UNLOCKED 0
#define LOCKED 1
#define LOCKED_2 2
#define LOCKED_3 3 
#define LOCKED_4 4 // leading tones and secondary leading tones
#define LOCKED_MAX 5 // common tones

static int hold_up = 1;
float samples_per_beat = 0;

enum Note
{
    A = 0,
    As = 1,
    Bf = 1,
    B = 2,
    Cf = 2,
    Bs = 3,
    C = 3,
    Cs = 4,
    Df = 4,
    D = 5,
    Ds = 6,
    Ef = 6,
    E = 7,
    Ff = 7,
    Es = 8,
    F = 8,
    Fs = 9,
    Gf = 9,
    G = 10,
    Gs = 11,
    Af = 11
};

enum Solfege
{
    REST = 0,
    DO = 1,
    DI = 2,
    RE = 3,
    ME = 4,
    MI = 5,
    FA = 6,
    FI = 7,
    SO = 8,
    LE = 9,
    LA = 10,
    TE = 11,
    TI = 12,
    DOH = 13
};

char* solfege[14] = {"rest", "DO", "DI", "RE", "ME", "MI", "FA", "FI", "SO", "LE", "LA", "TE", "TI", "DOH"};
char* accidental[5] = {"flats", "flat", "sharps or flats", "sharp", "sharps"};
char* endianness[2] = {"big-endian", "little-endian"};
enum {ARG_EXEC, ARG_OUTFILE, NUM_ARGS};

static int total_beats = 0;
int global_mode; // Major or minor
int global_tempo;
int global_meter; 
int global_index;
int max_chords;
int print_chords;

int abort_me = 0;

int scales[NUM_MODES][SCALE_SIZE] = {
    {DO, RE, MI, FA, SO, LA, TI, DOH},
    {DO, RE, ME, FA, SO, LE, TE, DOH}
};

int fancy_change[NUM_MODES][SCALE_SIZE] = {
    {0, 0, -1, 1, 0, -1, -1, 0},
    {0, 0, 1, 1, 0, 1, 1, 0}
};

typedef struct Chord
{
    int root;
    int bass;
    int inversion[2];
    int mode;
    int type;
    int isSpecial;
    int targets[2];
    int index;
    int notes[4];
} chord;

typedef struct Chord_Block
{
    chord chord_data;
    int start_measure;
    int start_count;
    int num_beats;
    int fancy;

} chord_block;

typedef struct NewNote
{
    int value;
    float length;
    int start_measure;
    int start_beat;
    int keep_gap;
    
} newNotes;

typedef struct Breakpoint
{
    float frequency;
    int num_frames;
    float gap;
    unsigned long int start;
    int downbeat;
} breakpoint;

typedef struct Target 
{
    int type;
    int mode;
    int index;
    int measure;
    int count;
    int beats;
} target;

enum Chord_Indices
{
    //TONIC
    I = 0,
    I6 = 1,
    //SUBDOMINANT
    VI = 0,
    IV = 1,
    II6 = 2,
    VofV = 3, 
    II4_3 = 3, 
    JUNK = 4,
    //DOMINANT
    V6 = 0,
    VII4_2 = 1,
    VII6 = 2,
    V7 = 3,
    V = 4
};

int num_choices_in_type[NUM_TYPES] = {NUM_TONIC_CHORDS, NUM_SUBDOMINANT_CHORDS, NUM_DOMINANT_CHORDS};

chord chord_options[NUM_TYPES][NUM_MODES][NUM_OPTIONS] = {
    {
        {
            {DO, DO, {5,3}, MAJOR, TONIC, 0, {0,0}, I, {DO, MI, SO, DO}}, 
            {DO, MI, {6,3}, MAJOR, TONIC, 0, {0,0}, I6, {MI, DO, SO, MI}}, 
            {DO, DO, {5,3}, MAJOR, TONIC, 0, {0,0}, I, {DO, SO, MI, DO}}, 
            {DO, MI, {6,3}, MAJOR, TONIC, 0, {0,0}, I6, {MI, DO, SO, MI}},
            {DO, DO, {5,3}, MAJOR, TONIC, 0, {0,0}, I, {DO, SO, MI, DO}}
        }, 
        {
            {DO, DO, {5,3}, MINOR, TONIC, 0, {0,0}, I, {DO, ME, SO, DO}}, 
            {DO, ME, {6,3}, MINOR, TONIC, 0, {0,0}, I6, {ME, DO, SO, ME}}, 
            {DO, DO, {5,3}, MINOR, TONIC, 0, {0,0}, I, {DO, SO, ME, DO}}, 
            {DO, ME, {6,3}, MINOR, TONIC, 0, {0,0}, I6, {ME, DO, SO, ME}},
            {DO, DO, {5,3}, MINOR, TONIC, 0, {0,0}, I, {DO, SO, ME, DO}}
        }
    },
    {
        {
            {LA, -LA, {5,3}, MINOR, SUBDOMINANT, 0, {0,0}, VI, {LA, MI, DO, -LA}}, 
            {FA, FA, {5,3}, MAJOR, SUBDOMINANT, 0, {0,0}, IV, {LA, FA, DO, FA}}, 
            {RE, FA, {6,3}, MINOR, SUBDOMINANT, 0, {0,0}, II6, {LA, FA, RE, FA}}, 
            {RE, RE, {5,3}, MAJOR, SUBDOMINANT, 2, {V,V}, VofV, {FI, RE, LA, RE}},
            {JUNK, JUNK, {JUNK, JUNK}, JUNK, SUBDOMINANT, 4, {JUNK, JUNK}, JUNK, {JUNK, JUNK, JUNK, JUNK}}
        },
        {
            {LE, -LE, {5,3}, MAJOR, SUBDOMINANT, 0, {0,0}, VI, {LE, ME, DO, -LE}}, 
            {FA, FA, {5,3}, MINOR, SUBDOMINANT, 0, {0,0}, IV, {LE, FA, DO, FA}}, 
            {RE, FA, {6,3}, DIMINISHED, SUBDOMINANT, 0, {0,0}, II6, {LE, FA, RE, FA}}, 
            {RE, -LE, {4,3}, HALF_DIMINISHED_7, SUBDOMINANT, 2, {V7, V7}, II4_3, {DO, FA, RE, -LE}},
            {JUNK, JUNK, {JUNK, JUNK}, JUNK, SUBDOMINANT, 4, {JUNK, JUNK}, JUNK, {JUNK, JUNK, JUNK, JUNK}}
        }
    },
    {
        {
            {SO, -TI, {6,3}, MAJOR, DOMINANT, 3, {V, I}, V6, {SO, RE, SO, -TI}}, 
            {TI, -LA, {4,2}, HALF_DIMINISHED_7, DOMINANT, 1, {V7, V7}, VII4_2, {FA, TI, RE, -LA}}, 
            {TI, RE, {6,3}, DIMINISHED, DOMINANT, 0, {0,0}, VII6, {FA, RE, TI, RE}}, 
            {SO, -SO, {7,5}, DOMINANT_7, DOMINANT, 0, {0,0}, V7, {TI, FA, RE, -SO}},
            {SO, SO, {5,3}, MAJOR, DOMINANT, 0, {0,0}, V, {TI, SO, RE, SO}}
        },
        {
            {SO, -TI, {6,3}, MAJOR, DOMINANT, 3, {V, I}, V6, {SO, RE, SO, -TI}}, 
            {TI, -LE, {4,2}, FULLY_DIMINISHED_7, DOMINANT, 1, {V7, V7}, VII4_2, {FA, TI, RE, -LE}}, 
            {TI, RE, {6,3}, DIMINISHED, DOMINANT, 0, {0,0}, VII6, {FA, RE, TI, RE}}, 
            {SO, -SO, {7,5}, DOMINANT_7, DOMINANT, 0, {0,0}, V7, {TI, FA, RE, -SO}},
            {SO, SO, {5,3}, MAJOR, DOMINANT, 0, {0,0}, V, {TI, SO, RE, SO}}
        }
    }
};

//chord ** chord_options[3] = {tonic_chords, subdominant_chords, dominant_chords};