// ////////////////////////////
// functions.h
// ////////////////////////////

#include "variables2.h"

/* thanks to the SNDAN programmers for this */
/* return 0 for big-endian machine, 1 for little-endian machine*/
/* so we can tell user what order the data is */ 
// Wasn't sure if I needed to cite the source for this function. 
//I literally copied and pasted byte_order() from a homework assignment, complete with the comment.
int byte_order()					
{						    
  int one = 1;
  char * endptr = (char *) &one;
  return (* endptr);
}

int getSign(int num)
{
    if(num > 0)
        return 1;
    else if(num < 0)
        return -1;
    else
        return 0;
}

int getTonic(int index, int isMinor)
{
    int keys[2][15] = {
        {Cf, Gf, Df, Af, Ef, Bf, F, C, G, D, A, E, B, Fs, Cs},
        {Af, Ef, Bf, F, C, G, D, A, E, B, Fs, Cs, Gs, Ds, As}
    };

    return keys[isMinor][index];
}

float getFreqFromValue(const float f0, int value)
{
    if(value == REST)
        return 0;
    else
    {
        float exponent = ((float)(value-1)/12.0);
        float new_freq = f0 * pow(2.0, exponent);
        return new_freq;
    }
    
}

int mod_scale(int note) // returns ((abs(note) - 1) % OCTAVE + 1)
{
    if(note == 0)
        return 0;
    else 
        return ((abs(note) - 1) % OCTAVE + 1);
    
}

int getScaleIndex(int value) // returns between 0 (DO) and 7 (DOH)
{
    int positive[13] = {0, 0, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7}; // DO, DI, RE, ME, MI, FA, FI, SO, LE, LA, TE, TI, DOH
    
    int index = positive[mod_scale(value) - 1];
    return index;
}

int subtractScaleIndices(int i1, int i2, int get_shortest_distance)
{
    int diff = i1 - i2;
    int alt_diff = (abs(diff) - 7) * getSign(diff);

    if(get_shortest_distance == 0 || abs(diff) < abs(alt_diff))
        return diff;
    else
        return alt_diff;
}

char* printkey(int index, int isMinor) // Generates the name of a key based on its index and major/minor
{
    if(index % 15 != index)
        return "INVALID!";

    char key1[7] = {'C', 'G', 'D', 'A', 'E', 'B', 'F'};
    char key2[3] = {'b', ' ', '#'};
    char* key3[2] = {" major", " minor"};

    int trueIndex = index + 3*isMinor;

    static char myKey[9];
    int i = trueIndex % 7;

    myKey[0] = key1[i];

    if(trueIndex < 6)
        myKey[1] = key2[0];
    else if(trueIndex < 13)
        myKey[1] = key2[1];
    else
        myKey[1] = key2[2];

    for(int j = 0; j < 6; j++)
        myKey[j+2] = key3[isMinor][j];

    return myKey;
}

float combineSamples(unsigned long int sample_num, breakpoint * soprano, breakpoint * alto, breakpoint * tenor, breakpoint * bass, int * num_notes) // This function determines the appropriate contribution from each voice at a particular sample, combines the samples and outputs the result.
{
    breakpoint * breakpoints[4] = {soprano, alto, tenor, bass};
    static int downbeat_index = 0;
    static int downbeat_index2 = 0;
    static unsigned long int start[4] = {0, 0, 0, 0};
    static unsigned long next_start[4] = {0, 0, 0, 0};
    static int indices[4] = {0, 0, 0, 0};
    float amps[4] = {0.0, 0.0, 0.0, 0.0};

    if(sample_num == 0)
    {
        for(int i = 0; i < 4; i++)
        {
            if(num_notes[i] == 0)
                continue;

            indices[i] = 0;
            start[i] = 0;
            next_start[i] = breakpoints[i][1].start;
            downbeat_index = 0;
            downbeat_index2 = 0;
        }
    }

    for(int i = 0; i < 4; i++)
    {   
        if(num_notes[i] <= indices[i])
        { 
            amps[i] = 0;
            continue;
        }
        
        if(sample_num >= next_start[i])
        {
            indices[i]++;
            start[i] = next_start[i];
            if(indices[i] < num_notes[i]-1) 
                next_start[i] = breakpoints[i][indices[i]+1].start;
            else 
                next_start[i] = breakpoints[i][indices[i]].start + breakpoints[i][indices[i]].num_frames;
        }

        if(sample_num >= next_start[i] - breakpoints[i][indices[i]].gap)
            continue;

        float freq = breakpoints[i][indices[i]].frequency; // oscillations per second
        if(freq < 1)
        {
            amps[i] = 0;
            continue;
        }
        
        int sample_difference = sample_num - start[i];
        int samples_per_1000_oscillations = 1000*RATE/freq;
        int big_remainder = (1000*sample_difference) % samples_per_1000_oscillations;
        float remainder = big_remainder/1000;

        amps[i] = remainder * freq/RATE;

        if(amps[i] > 0.5)
            amps[i] -= 1;
    }

    float total_amp = 0;
    int power = -1;
    
    for(int i = 0; i < NUM_VOICES; i++)
    {
        if(num_notes[BASS] == 0)
            break;
        else if(breakpoints[i][indices[i]].downbeat == 1)
            amps[i] *= 1.1;
        else
            amps[i] *= 0.9;
            
    }
    
    for(int i = 0; i < NUM_VOICES; i++)
    {
        total_amp += amps[i];
        if(num_notes[i] > 0)
            power++;
    }

    total_amp *= pow(0.7, (float)power);

    return total_amp;
    
}

int obtainParameters()
{
    int random = 0;
    for(int i = 0; i < 4; i++)
        printf("\n\n\n\n\n\n\n\n");

    printf("Enter an integer between -7 and 7 to determine the key signature, \n8 to randomize key, meter, and tempo,\nor 9 to quit...\n\n");

    static int j0 = 0;

    for(int i = 0; i < 15; i++)
    {
        char m1[9];
        strcpy(m1, printkey(i, 0));
        char m2[9];
        strcpy(m2, printkey(i, 1));

        if(abs(i-7) <= 1 || i == 9)
            j0++;

        printf("\n\t%2d = %s / %s (%d %s)\n", i-7, m1, m2, abs(i-7), accidental[j0]); // Print the list of possible key signatures
    }

    printf("\n\nEnter here: ");
    int valid = scanf("%d", &global_index);

    while(valid != 1 || abs(global_index) > 7)
    {
        if(global_index == 9)
        {
            printf("Goodbye...\n");
            return 1;
        }
        else if(global_index == 8)
        {
            random = 1;
            global_index = rand() % 15 - 7;
            break;
        }

        printf("\nInvalid response. Please try again.\n");
        printf("Enter an integer between -7 and 7 to determine the key signature, \n8 to randomize key, meter, and tempo,\nor 9 to quit...\n\n");

        j0 = 0;

        for(int i = 0; i < 15; i++)
        {
            char m1[9];
            strcpy(m1, printkey(i, 0));
            char m2[9];
            strcpy(m2, printkey(i, 1));

            if(abs(i-7) <= 1 || i == 9)
                j0++;

            printf("\n\t%2d = %s / %s (%d %s)\n", i-7, m1, m2, abs(i-7), accidental[j0]); // Print the list of possible key signatures
        }

        printf("\nEnter here: ");
        valid = scanf("%d", &global_index);
    }

    global_index += 7;
    char m1[9];
    strcpy(m1, printkey(global_index, 0));
    char m2[9];
    strcpy(m2, printkey(global_index, 1));

    
    if(random == 0)
    {
        printf("\nYou selected %s / %s. \n\nEnter 0 for %s, 1 for %s, \n8 to randomly select major or minor, \nor 9 to quit: ", m1, m2, m1, m2); 
        valid = scanf("%d", &global_mode);
    }
    else
        global_mode = rand() % 2;

    while(valid != 1 || global_mode != global_mode % 2)
    {
        if(global_mode == 9)
        {
            printf("Goodbye...\n");
            return 1;
        }
        else if(global_mode == 8)
        {
            global_mode = rand() % 2;
            break;
        }

        printf("Invalid input. \n\nEnter 0 for %s, 1 for %s, \n8 for a random selection, \nor 9 to quit: ", m1, m2); 
        valid = scanf("%d", &global_mode);
    }

    if(random == 0)
    {
        printf("\nWhat is the meter? \n\nEnter 2 for 2/4, 3 for 3/4, or 4 for 4/4, \nenter 8 for random meter and tempo, \nor enter 9 to quit: ");
        valid = scanf("%d", &global_meter);
    }
    else
        global_meter = rand() % 3 + 2;

    while(valid != 1 || global_meter < 2 || global_meter > 4)
    {
        if(global_meter == 9)
        {
            printf("Goodbye...\n");
            return 1;
        }
        else if(global_meter == 8)
        {
            random = 1;
            global_meter = rand() % 3 + 2;
            break;
        }

        printf("Invalid input. \n\nEnter 2 for 2/4, 3 for 3/4, or 4 for 4/4, \nenter 8 for a random meter and tempo, \nor enter 9 to quit: "); 
        valid = scanf("%d", &global_meter);
    }

    if(random == 0)
    {
        printf("\nWhat is the tempo (bpm)? \n\nEnter an integer between 40 and 240, \nenter 8 for random, \nor enter 9 to quit: ");
        valid = scanf("%d", &global_tempo);
    }
    else
        global_tempo = rand() % 201 + 40;

    while(valid != 1 || global_tempo < 40 || global_tempo > 240)
    {
        if(global_tempo == 9)
        {
            printf("Goodbye...\n");
            return 1;
        }
        else if(global_tempo == 8)
        {
            random = 1;
            global_tempo = rand() % 201 + 40;
            break;
        }

        printf("Invalid input. \n\nEnter an integer between 40 and 240, \nenter 8 for random, \nor enter 9 to quit: "); 
        valid = scanf("%d", &global_tempo);
    }

    if(random == 1)
    {
        printf("You selected: Random\n\tKey: %s\n\tMeter: %d/4\n\tTempo: %d bpm\n\n", printkey(global_index, global_mode), global_meter, global_tempo);
        printf("Is this okay? Enter 9 to quit, or any other key to continue: ");
        scanf("%d", &random);

        if(random == 9)
        {
            printf("\nQuitting... goodbye!\n");
            return 1;
        }
    }

    printf("\nWhat is the maximum length in minutes? \nEnter between 1 and 5, or enter 9 to quit: ");

    int max_minutes = 0;
    valid = scanf("%d", &max_minutes);
    
    while(valid != 1 || max_minutes < 1 || max_minutes > 5)
    {
        if(max_minutes == 9)
        {
            printf("\nGoodbye...\n");
            return 1;
        }
        printf("\nInvalid input. \nEnter between 1 and 5, or enter 9 to quit: "); 

        valid = scanf("%d", &max_minutes);
    }

    max_chords = global_tempo * max_minutes;
    int min_chords = 40 * global_meter;

    max_chords = fmax(min_chords, max_chords);
    if(max_chords != global_tempo * max_minutes)
    {
        printf("Maximum number of chords cannot be less than %d. \nNew max length is %.2f minutes...\n", min_chords, (float)max_chords/(float)global_tempo);
    }

    printf("\nPrint chords? \nEnter 0 for no, 1 for yes, or 9 to quit: ");

    valid = scanf("%d", &print_chords);
    
    while(valid != 1 || print_chords < 0 || print_chords > 1)
    {
        if(print_chords == 9)
        {
            printf("\nGoodbye...\n");
            return 1;
        }
        printf("\nInvalid input. \nEnter 0 for no, 1 for yes, or 9 to quit: "); 

        valid = scanf("%d", &print_chords);
    }

    return 0;
}

int convertLowNotes(int note, int undo) // 0 for convert to linear, 1 for convert to mod_scale
{
    int output = note;

    if(undo == 1)
    {
        if(note == -1000)
            return REST;
        else if(note < DO)
        {
            output--;
            output %= OCTAVE;
            output++;
            output *= -1;
            return output;
        }
        else
            return note;
    }
    else if(undo == 0)
    {
        if(note == REST)
            return -1000;
        else if(note < 0)
        {
            output = mod_scale(note);
            if(note <= -DOH)
                output += OCTAVE;
            
            output -= OCTAVE;
            return output;
        }
        else
            return note;
    }
    else
        return note;
}

int addSemitones(int note, int i, int constrain)
{
    if(note == REST)
        return i;
    else if(i == 0)
        return note;
    else
    {
        int new_note;
        if(note > 0)
        {
            new_note = note + i;
            if(new_note <= 0)
            {
                while(new_note <= 0)
                    new_note += OCTAVE;

                new_note *= getSign(i);
            }
        }
        else if(note < 0)
        {
            new_note = note - i;
            if(new_note >= 0)
            {
                while(new_note >= 0)
                    new_note -= OCTAVE;

                new_note *= -1*getSign(i);
            }
        } 

        if(constrain == 1 || new_note < -DOH)
        {
            while((new_note > -FI && new_note < REST) || new_note > MI+OCTAVE || new_note < -DOH)
            {
                if(new_note > -FI && new_note < REST) 
                    new_note *= -1;
                else
                    while(new_note < -DOH)
                        new_note += OCTAVE;
                
                if(new_note == -DOH)
                    new_note = DO;
                if(new_note > MI + OCTAVE)
                {
                    new_note = mod_scale(new_note);
                    if(new_note == DO)
                        new_note = DOH;
                }
            }
        }

        return new_note;
    }
}

int get_distance(int note1, int note2) // Returns note1 - note2
{
    if(note1 == REST || note2 == REST)
        return 1000;
    else
    {
        int d1, d2;

        if(note1 > REST)
            d1 = note1 + OCTAVE;
        else
            d1 = abs(note1);

        if(note2 > REST)
            d2 = note2 + OCTAVE;
        else
            d2 = abs(note2);

        return (d1 - d2);
    }
    
}

int count_times(int chord[NUM_VOICES], int note)
{
    int num_times = 0;
    for(int i = 0; i < NUM_VOICES; i++)
    {
        if(mod_scale(chord[i]) == mod_scale(note) != 0)
            num_times++;
    }

    return num_times;
}

int searchWithin(int chord[NUM_VOICES], int note, int num_appearance)
{
    int num_times = 0;
    for(int i = 0; i < NUM_VOICES; i++)
    {
        if(mod_scale(chord[i]) == mod_scale(note) != 0)
            num_times++;

        if(num_times == num_appearance)
            return i;
    }

    return -1;
}
int * randomSort(int old_array[NUM_VOICES], int locked_nums[NUM_VOICES])
{
    int temp_array[NUM_VOICES] = {-1, -1, -1, -1};
    int num_unlocked = NUM_VOICES;
    for(int i = 0; i < NUM_VOICES; i++)
    {
        if(locked_nums[i] < LOCKED_2)
            temp_array[i] = i;
        else
            num_unlocked--;
    }

    int new_indices[NUM_VOICES] = {SOPRANO, ALTO, TENOR, BASS};

    for(int i = num_unlocked; i > 0; i--)
    {
        int j = rand() % i;
        int k = 0;
        int l = 0;

        for(l = 0; l < NUM_VOICES; l++)
        {
            if(temp_array[l] == -1)
                continue;
            else if(k == j)
                break;
            else
                k++;
        }

        int j2 = rand() % i;
        int k2 = 0;
        int l2 = 0;

        for(l2 = 0; l2 < NUM_VOICES; l2++)
        {
            if(temp_array[l2] == -1)
                continue;
            else if(k2 == j2)
                break;
            else
                k2++;
        }
        if(l >= NUM_VOICES || l2 >= NUM_VOICES)
            break;

        int old_l = new_indices[l];
        new_indices[l] = new_indices[l2];
        new_indices[l2] = old_l;
        temp_array[l] = -1;
    }

    static int new_array[NUM_VOICES] = {0, 0, 0, 0};
    for(int i = 0; i < NUM_VOICES; i++)
        new_array[i] = old_array[new_indices[i]];

    return new_array;
}

int countPerfectIntervals(int * old_chord, int * new_chord)
{
    int perfects = 0;

    for(int i = 0; i < NUM_VOICES; i++)
    {
        for(int j = 0; j < NUM_VOICES; j++)
        {
            if(mod_scale(old_chord[i])-mod_scale(old_chord[j]) == SO-DO && mod_scale(new_chord[i])-mod_scale(new_chord[j]) == SO-DO && i > j)
                perfects++;
            else if(mod_scale(old_chord[i])-mod_scale(old_chord[j]) == FA-DO && mod_scale(new_chord[i])-mod_scale(new_chord[j]) == FA-DO && i < j)
                perfects++;
            else if(mod_scale(old_chord[i])-mod_scale(old_chord[j]) == 0 && mod_scale(new_chord[i])-mod_scale(new_chord[j]) == 0 && i != j)
                perfects++;
        }
    }

    return perfects;
}

int getPerfectCadence(chord_block old_block, chord_block new_block)
{
    if(old_block.chord_data.type != DOMINANT)
    {
        return 0;
    }
    else if(new_block.chord_data.type != TONIC)
    {
        return 0;
    }
    else if(abs(old_block.chord_data.notes[BASS]) != SO || abs(old_block.chord_data.notes[SOPRANO]) != TI)
        return 0;
    else if(new_block.chord_data.notes[BASS] != DO || mod_scale(new_block.chord_data.notes[SOPRANO]) != DO)
        return 0;
    else if(old_block.chord_data.notes[SOPRANO] == TI && new_block.chord_data.notes[SOPRANO] != DOH)
        return 0;
    else if(old_block.chord_data.notes[SOPRANO] == -TI && new_block.chord_data.notes[SOPRANO] != DO && new_block.chord_data.notes[SOPRANO] != -DOH)
        return 0;
    else if((count_times(old_block.chord_data.notes, SO) < 2 && count_times(old_block.chord_data.notes, FA) != 1 && count_times(old_block.chord_data.notes, SO) != 1 ) || count_times(new_block.chord_data.notes, DO) < 2)
    {
        return 0;
    }
    else if(count_times(old_block.chord_data.notes, TI) != 1 || count_times(new_block.chord_data.notes, scales[global_mode][2]) != 1)
        return 0;
    else
    {
        for(int i = DO; i < DOH; i++)
        {
            if(i == SO)
                continue;
            else if(count_times(new_block.chord_data.notes, i) > 0)
            {
                if(i != DO && i != SO && i != scales[global_mode][2])
                    return 0;
                else
                    continue;
            }
            else if(count_times(old_block.chord_data.notes, i) > 0)
            {
                if(i != RE && i != SO && i != FA && i != TI)
                    return 0;
                else if(i == FA && old_block.chord_data.index != V7)
                    return 0;
                else
                    continue;
            }
        }

        return 1;
    }
}

int * voiceLead(chord old_chord_data, chord temp_chord_data)
{
    int old_chord[NUM_VOICES]; 
    int temp_chord[NUM_VOICES];
    
    for(int i = 0; i < NUM_VOICES; i++)
    {
        old_chord[i] = old_chord_data.notes[i];
        temp_chord[i] = temp_chord_data.notes[i];
    } 

    int locked_nums[NUM_VOICES] = {UNLOCKED, UNLOCKED, UNLOCKED, UNLOCKED}; 

    static int new_chord[NUM_VOICES] = {0, 0, 0, 0};
    for(int i = 0; i < NUM_VOICES; i++)
        new_chord[i] = temp_chord[i];

    int app_num = 1;

    for(int i = SOPRANO; i < NUM_VOICES; i++)
    {
        app_num = 1;
        while(count_times(temp_chord, old_chord[i]) >= app_num && (i != SOPRANO || ((old_chord[i] == TI || old_chord[i] == FI) && (temp_chord_data.root != old_chord_data.root || old_chord_data.type != temp_chord_data.type)) || rand() % 7 < 3))
        {
            if(searchWithin(temp_chord, old_chord[i], app_num++) == i)
            {
                new_chord[i] = old_chord[i];
                locked_nums[i] = LOCKED_MAX; // keep common tones
            }
        }
    }

    app_num = 1;
    while(count_times(old_chord, TI) >= app_num && count_times(temp_chord, DO) >= app_num)
    {
        int res_num = searchWithin(old_chord, TI, app_num++); // resolution number
        
        if(locked_nums[res_num] >= LOCKED_MAX)
            continue;
        else if(old_chord[res_num] == -TI)
            new_chord[res_num] = DO;
        else
            new_chord[res_num] = DOH;

        locked_nums[res_num] = LOCKED_4;
    }

    app_num = 1;
    while(count_times(old_chord, FI) >= app_num && count_times(temp_chord, SO) >= app_num)
    {
        int res_num = searchWithin(old_chord, FI, app_num++); // resolution number

        if(locked_nums[res_num] >= LOCKED_MAX)
            continue;
        else if(old_chord[res_num] == -FI)
            new_chord[res_num] = -SO;
        else
            new_chord[res_num] = SO;

        locked_nums[res_num] = LOCKED_4;
    }

    int num_attempts = 0;

    int seventh_index = -1;
    if(old_chord_data.mode >= HALF_DIMINISHED_7)
    {
        int seventh = 0;
        if(old_chord_data.mode != FULLY_DIMINISHED_7)
            seventh = mod_scale(addSemitones(old_chord_data.root, TE-DO, 0));
        else
            seventh = mod_scale(addSemitones(old_chord_data.root, LA-DO, 0));

        seventh_index = searchWithin(old_chord, seventh, 1);
        if(seventh_index != -1)
            if(count_times(temp_chord, seventh) == 0)
            {
                new_chord[seventh_index] = seventh;
                do
                {
                    new_chord[seventh_index] = addSemitones(new_chord[seventh_index], -1, 0);
                    if(mod_scale(new_chord[seventh_index]) == seventh)
                        new_chord[seventh_index] = mod_scale(new_chord[seventh_index]);

                } while (new_chord[seventh_index] != seventh && count_times(temp_chord, new_chord[seventh]) == 0);
                
                if(new_chord[seventh_index] == seventh)
                    seventh_index = -1;
                else
                    locked_nums[seventh_index] = LOCKED_3;
            }
    }
    int new_chord_copy[NUM_VOICES];
    for(int i = 0; i < NUM_VOICES; i++)
        new_chord_copy[i] = new_chord[i];

    int num_locked = 0;
    num_attempts = 0;
    int num_lightly_locked = 4;
    int zero_crossing[NUM_VOICES] = {0, 0, 0, 0};
    int new_notes[NUM_VOICES] = {0, 0, 0, 0};

    while(num_locked < NUM_VOICES && num_attempts++ < 50)
    {
        for(int i = SOPRANO; i < NUM_VOICES; i++)
            new_chord[i] = new_chord_copy[i];

        for(int i = 0; i < NUM_VOICES; i++)
            zero_crossing[i] = 0;

        for(int i = 0; i < 13; i++)
        {
            if(i == 5 && num_attempts % 2 == 0)
                continue;

            int mod = 4;
            if(num_attempts > 10 && num_attempts < 20)
                mod = rand() % 5;
            else if(num_attempts >= 20)
                mod = rand() % 3;

            num_locked = 0;
            num_lightly_locked = 4;
    
            for(int j = 0; j < NUM_VOICES; j++)
            {
                if(locked_nums[j] != UNLOCKED)
                {
                    num_locked++;
                    if(locked_nums[j] > LOCKED)
                        num_lightly_locked--;

                    continue;
                }
                
                int new_note = mod_scale(abs(old_chord[j])+i);

                if(count_times(new_chord, new_note) < (count_times(temp_chord, new_note)) && new_note < SO + OCTAVE)
                {
                    if(rand() % 6 > mod || mod == 1)
                    {
                        new_note = new_chord[j];
                        mod *= 5;
                        if(mod == 0)
                            mod++;
                    }
                    else
                    {
                        if(abs(get_distance(new_note, old_chord[j])) > abs(get_distance(-new_note, old_chord[j])))
                            new_chord[j] = -new_note;
                        else if(abs(get_distance(new_note, old_chord[j])) > abs(get_distance(OCTAVE + abs(new_note), old_chord[j])))
                            new_chord[j] = OCTAVE + abs(new_note);
                        else
                            new_chord[j] = new_note;
                            
                        locked_nums[j] = LOCKED;
                        continue;
                    }
                }
                else
                    new_note = new_chord[j];

                if(abs(new_chord[j]) == i)
                    zero_crossing[j] = 1;

                new_note = mod_scale((abs(new_chord[j]) + 12 - i));

                if(count_times(new_chord, new_note) < (count_times(temp_chord, new_note)) && convertLowNotes(new_note, 0) >= SO - OCTAVE)
                {
                    if(rand() % 5 >= mod || mod == 1)
                    {
                        new_note = new_chord[j];
                        mod *= 5;
                        if(mod == 0)
                            mod++;
                    }
                    else
                    {
                        if(abs(get_distance(new_note, old_chord[j])) > abs(get_distance(-new_note, old_chord[j])))
                            new_chord[j] = -new_note;
                        else if(abs(get_distance(new_note, old_chord[j])) > abs(get_distance(OCTAVE + abs(new_note), old_chord[j])))
                            new_chord[j] = OCTAVE + abs(new_note);
                        else
                            new_chord[j] = new_note;

                        if((j == SOPRANO || (j < BASS && rand()%5 < 2)) && mod_scale(old_chord[j]) == RE && i == 5)
                            new_chord[j] = LA;

                        locked_nums[j] = LOCKED;
                        continue;
                    }
                }
                else
                    new_note = new_chord[j];
            }
        }

        for(int i = 0; i < NUM_VOICES; i++)
            new_notes[i] = convertLowNotes(new_chord[i], 0);

        int uh_oh = 0;
        for(int i0 = SOPRANO; i0 <= NUM_VOICES; i0++)
        {
            if(uh_oh > 0)
            {
                for(int i = SOPRANO; i < NUM_VOICES; i++)
                {
                    locked_nums[i] *= (locked_nums[i] > LOCKED);
                    new_chord[i] = new_chord_copy[i];
                }

                num_locked = 0;    
                break;
            }
            else if(i0 == NUM_VOICES || num_attempts == 50)
                break;

            for(int j0 = i0; j0 < NUM_VOICES; j0++)
            {
                if(uh_oh > 0)
                    break;
                else if(j0 == i0)
                    continue;
                else if(new_notes[j0] + OCTAVE * (j0-i0) > new_notes[i0])
                    uh_oh = 1;
            }

            if(uh_oh > 0)
                continue;
            
            for(int j0 = i0; j0 >= SOPRANO; j0--)
            {
                if(uh_oh > 0)
                    break;
                else if(j0 == i0)
                    continue;
                else if(new_notes[j0] - OCTAVE * (i0-j0) > new_notes[i0])
                    uh_oh = 1;
            }
        }
    }

    num_attempts = 0;
    do
    {
        if(num_attempts > 0)
        {
            for(int i = 0; i < NUM_VOICES; i++)
                new_chord[i] = new_chord_copy[i];
        }
        
        int * changed_chord = randomSort(new_chord, locked_nums);

        for(int i = 0; i < NUM_VOICES; i++)
            new_chord_copy[i] = new_chord[i];

        for(int i = 0; i < NUM_VOICES; i++)
            new_chord[i] = changed_chord[i];

        if(new_chord[BASS] >= FI && old_chord[BASS] <= MI && num_attempts <= 12)
            new_chord[BASS] *= -1;
        else if(1)
            continue;
        else if(addSemitones(new_chord[BASS], 10, 0) == old_chord[BASS] || addSemitones(new_chord[BASS], 11, 0) == old_chord[BASS])
            new_chord[BASS] = addSemitones(new_chord[BASS], OCTAVE, 1);
        else if(new_chord[BASS] == old_chord[BASS])
            new_chord[BASS] = addSemitones(new_chord[BASS], -12, 1);

    } while(countPerfectIntervals(old_chord, new_chord) > 0 && count_times(new_chord, TI) > 1 && count_times(new_chord, FI) > 1 && num_attempts++ < 25); // No parallel fifths

    
    for(int i = 0; i < NUM_VOICES; i++)
    {
        if(old_chord[i] == TI && mod_scale(new_chord[i]) == DO)
        {
            new_chord[i] = DOH;
            //printf("new_chord[%d] == %s\n", i, solfege[new_chord[i]]);
        }
        else if(old_chord[i] == -TI && mod_scale(new_chord[i]) == DO)
            new_chord[i] = DO;
    }

    for(int i = SOPRANO; i < NUM_VOICES; i++)
    {
        int j;
        for(j = 0; j <= 8; j++)
        {
            if(j == 8)
                break;
            else if(mod_scale(new_chord[i]) == scales[global_mode][j] || mod_scale(new_chord[i]) == TI || mod_scale(new_chord[i]) == FI)
                break;
        }
        if(j == 8)
        {
            new_chord[i] = scales[global_mode][abs(getScaleIndex(new_chord[i]))];
            printf("Off the scale!!!\n");
        }
    }
    
    return new_chord;
}

int limitRange(float frequency, int voice)
{
    float low_bass = getFreqFromValue(55, SO);
    float high_tenor = getFreqFromValue(A3, FA);
    float low_alto = getFreqFromValue(A3, RE);
    float high_soprano = getFreqFromValue(2*A4, TI);

    float limits[NUM_VOICES][2] = {
        {low_alto, high_soprano},
        {A3, A4},
        {110, high_tenor},
        {low_bass, A3}
    };

    if(frequency <= 1)
        return 0;
    else if(frequency < limits[voice][0])
        return 1;
    else if(frequency > limits[voice][1])
        return -1;
    else
        return 0;
}