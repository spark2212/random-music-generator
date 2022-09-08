// /////////////////////////////////////
// File:    Follow_My_Voice_Leading9.c
// Author:  Theodore Shore
// Date:    12/16/2020
//
// This program can either generate
// a four-part harmony in solfege 
// in various keys, meters, and
// tempos, and output the samples
// to a RAW file for playback.
// /////////////////////////////////////

#include "functions_10.h"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Usage: %s outfile.raw\n",argv[ARG_EXEC]);
        return 1;
    }

    srand(time(0));
    if(obtainParameters() == 1)
        return 0;
    
    printf("Generating song...\n");
    int num_tries = 0;
    while(num_tries++ < MAX_TRIES && abort_me == 0)    
    {
        printf("This is attempt number %d.\n", num_tries);
        int count = 0;
        int measure = 1;
        int num_chords = 0;

        total_beats = 0;
        
        count++;
        num_chords++;

        int switch_nums[4] = {global_meter-1, 1 + (global_meter%2), global_meter-1, global_meter-1};

        int switch_num = 3;
        int switch_count = switch_nums[switch_num];
        int forced_to_index_0 = 0;
        int cooldown = 0;

        if(global_meter == 4)
            switch_nums[0] = 1;

        chord_block * chord_blocks = (chord_block*) malloc(sizeof(chord_block));
        chord_blocks[0].chord_data = chord_options[TONIC][global_mode][I];
        chord_blocks[0].start_measure = 0;
        chord_blocks[0].start_count = count;

        int last_three_limits[THREE] = {0, 0, 0};
        last_three_limits[2] = switch_nums[switch_num];

        while(num_chords < max_chords && abort_me == 0)
        {  
            num_chords++;
            count %= global_meter;
            if(count++ == 0)
                measure++;
            
            int type_num = chord_blocks[num_chords-2].chord_data.type;
            int index_num = chord_blocks[num_chords-2].chord_data.index;
            int special_num = chord_blocks[num_chords-2].chord_data.isSpecial;
            int special_targets[2];

            for(int i = 0; i < 2; i++)
                special_targets[i] = chord_blocks[num_chords-2].chord_data.targets[i];

            int new_type = type_num;
            int new_index = index_num;
            
            if(switch_count-- == 0)
            {   
                if(type_num == TONIC)
                    new_type = rand() % 2 + 1; // results in either SUBDOMINANT or DOMINANT
                else if(type_num == SUBDOMINANT)
                    new_type = DOMINANT;
                else if(type_num == DOMINANT)
                    new_type = TONIC;

                if(forced_to_index_0 > 0)
                {
                    if(global_meter == 3 || special_num != 0)
                        new_type = (type_num+1)%NUM_TYPES;
                } 

                if(special_num != 0)
                {
                    new_type = (type_num+1)%NUM_TYPES;
                    new_index = special_targets[special_num%2];
                }
                else
                    new_index = rand() % (num_choices_in_type[new_type] - (new_type%2)); 
                
                if(switch_num == 3 && cooldown < 1)
                    switch_num = -1;
                else if(cooldown > 0)
                {   
                    switch_nums[0] = cooldown - 1;
                    switch_num = 0;
                }
                    
                int attempts = 0;
                do
                {
                    if(switch_num == 3 || cooldown > 0)
                        break;

                    do
                    {
                        switch_num = rand() % 3;
                        switch_num *= (switch_num < global_meter);
                    }while(switch_num == 0 && cooldown == 1);

                    if(attempts++ > 100)
                    {
                        abort_me = 1;
                        break;
                    }
                }
                while((switch_nums[switch_num]+count > global_meter && count != global_meter && forced_to_index_0 == 0));

                cooldown--;
                
                if(forced_to_index_0 != 0 && global_meter != 4)
                    switch_num = 0;
                else if(switch_num == 0 && new_type != TONIC && count == global_meter)
                    switch_num = 3;

                switch_count = switch_nums[switch_num];
                
                for(int i = 0; i < 2; i++)
                    last_three_limits[i] = last_three_limits[i+1];
                
                static int light_up = 0;
                if(measure % 4 == 0 && cooldown < 0)
                {
                    switch_num = 3;
                    switch_count = switch_nums[switch_num];
                }
                else if(switch_num == 0 && global_meter != 4 && cooldown < 0)
                {
                    light_up = 1;

                    if(forced_to_index_0 == 0)
                    {
                        if(global_meter != 4)
                            switch_count = global_meter-2;
                        else
                            switch_count = 3;

                        switch_nums[0] = 0;
                        forced_to_index_0 += (switch_count - 0);
                    }
                    else if(forced_to_index_0 == global_meter - 1)
                    {
                        if(global_meter == 3)
                            cooldown = 2;    

                        switch_count = switch_nums[0];
                        switch_nums[0] = global_meter - 1;
                    }
                    else
                        switch_count = switch_nums[0]; 
                        
                    forced_to_index_0++;
                    forced_to_index_0 %= global_meter;
                }
                else
                    light_up = 0;
                
                last_three_limits[2] = switch_count;
            }
            else
            {
                if(special_num == 1 || special_num == 3)
                {
                    int special_index = rand() % 2;
                    if(special_index == 0 || switch_count == 0)
                        new_index = special_targets[0];
                    else
                        new_index = index_num;
                }
                else if(special_num != 2)
                    new_index = rand() % (num_choices_in_type[type_num]-index_num) + index_num;
                
                if(switch_count > 1 && new_type == SUBDOMINANT && new_index == VofV)
                    new_index = index_num;
            }

            if(chord_options[new_type][global_mode][new_index].isSpecial == 1 && switch_count == 0)
                new_index = chord_options[new_type][global_mode][new_index].targets[0];
            
            chord_blocks = (chord_block*) realloc(chord_blocks, num_chords*sizeof(chord_block));
            chord_blocks[num_chords-1].chord_data = chord_options[new_type][global_mode][new_index];
            chord_blocks[num_chords-1].start_count = count;
            chord_blocks[num_chords-1].start_measure = measure;
            chord_blocks[num_chords-1].num_beats = QUARTER;
            chord_blocks[num_chords-1].fancy = 0; 

            int output_notes[NUM_VOICES];
            int * output_ptr = voiceLead(chord_blocks[num_chords-2].chord_data, chord_blocks[num_chords-1].chord_data);
            for(int i = 0; i < NUM_VOICES; i++)
                output_notes[i] = output_ptr[i]; 

            for(int i = 0; i < NUM_VOICES; i++)
                chord_blocks[num_chords-1].chord_data.notes[i] = output_notes[i];

            int PAC = getPerfectCadence(chord_blocks[num_chords-2], chord_blocks[num_chords-1]);
            int exit_trigger = 0;

            if(last_three_limits[1] == global_meter-1 || (last_three_limits[0] == global_meter-2))
                exit_trigger = 1;
 
            if(measure >= 32 && PAC == 1 && (last_three_limits[0] + last_three_limits[1] + 2) % global_meter == 0 && last_three_limits[2] == switch_nums[2] && exit_trigger == 1)
            {
                chord_blocks[num_chords-1].num_beats = 2*global_meter;
                total_beats += 2*global_meter;
                num_chords++;
                chord_blocks = (chord_block*) realloc(chord_blocks, num_chords*sizeof(chord_block));
                chord_blocks[num_chords-1].chord_data = chord_options[new_type][global_mode][new_index];
                for(int i = 0; i < NUM_VOICES; i++)
                    chord_blocks[num_chords-1].chord_data.notes[i] = REST;

                chord_blocks[num_chords-1].start_count = 0;
                chord_blocks[num_chords-1].start_measure = measure+2;
                chord_blocks[num_chords-1].num_beats = global_meter;
    
                break;      
            }
            else
                total_beats += QUARTER;
        }

        if(num_chords >= max_chords || abort_me != 0)
        {
            free(chord_blocks);
            if(num_tries == MAX_TRIES || abort_me != 0)
                printf("Too many attempts! Goodbye!\n");

            continue;
        }

        int num_notes[NUM_VOICES] = {1, 1, 1, 1};

        newNotes * bass_notes = (newNotes*) malloc(sizeof(newNotes));

        bass_notes[0].value = chord_blocks[0].chord_data.notes[BASS];
        bass_notes[0].length = QUARTER;
        bass_notes[0].start_measure = 0;
        bass_notes[0].start_beat = 0;
        bass_notes[0].keep_gap = 1;

        int start_beat = 0;
        int start_measure = 0;

        printf("Generating bass notes...\n");

        for(int i = 1; i < num_chords; i++)
        {
            if(++start_beat == global_meter)
            {
                start_beat %= global_meter;
                start_measure++;
            }

            num_notes[BASS]++;
            bass_notes = (newNotes*) realloc(bass_notes, num_notes[BASS]*sizeof(newNotes));
            bass_notes[num_notes[BASS]-1].value = chord_blocks[i].chord_data.notes[BASS];
            bass_notes[num_notes[BASS]-1].length = chord_blocks[i].num_beats;
            bass_notes[num_notes[BASS]-1].start_beat = start_beat;
            bass_notes[num_notes[BASS]-1].start_measure = start_measure;
            bass_notes[num_notes[BASS]-1].keep_gap = 1;

            if(i >= num_chords - 3)
                continue;
            else if(rand() % 13 < 3)
            {
                num_notes[BASS]++;
                bass_notes = (newNotes*) realloc(bass_notes, num_notes[BASS]*sizeof(newNotes));
                int old_note = bass_notes[num_notes[BASS]-2].value;
                int x = rand() % NUM_VOICES; int middle_note = chord_blocks[i].chord_data.notes[x];
                int next_note = chord_blocks[i+1].chord_data.notes[BASS];
                int keeper = 1;
                int sounder = (rand() % 10 > 2);
                if(middle_note == old_note)
                    keeper = (rand() % 10 > 2);
                
                if((mod_scale(middle_note) != FI && mod_scale(middle_note) != TI) || next_note == addSemitones(middle_note, 1, 0))
                {
                    float split = ((rand() % 8 < 3) + 2) * SIXTEENTH;
                    bass_notes[num_notes[BASS]-1].value = middle_note * sounder;
                    bass_notes[num_notes[BASS]-2].length = split;
                    bass_notes[num_notes[BASS]-1].length = QUARTER - split;
                    bass_notes[num_notes[BASS]-1].start_beat = start_beat;
                    bass_notes[num_notes[BASS]-1].start_measure = start_measure;
                    bass_notes[num_notes[BASS]-2].keep_gap = keeper;
                    bass_notes[num_notes[BASS]-1].keep_gap = (rand() % 5 > 1);

                    int old_index = getScaleIndex(old_note);
                    int alt = rand() % 7 - 1;

                    if(abs(alt) == 1 && chord_blocks[i].fancy < 2 && chord_blocks[i].chord_data.type == TONIC)
                    {
                        middle_note = scales[global_mode][(old_index + alt)%7];
                        if(old_index + alt < 0)
                            middle_note *= -1;
                        else if(old_index + alt >= 7)
                            middle_note += OCTAVE;

                        bass_notes[num_notes[BASS]-1].value = middle_note;

                        if(global_mode == MINOR && (mod_scale(middle_note) == TE || (mod_scale(middle_note) == LE && (mod_scale(chord_blocks[i+1].chord_data.root) == SO || mod_scale(next_note) == TI))))
                        {
                            bass_notes[num_notes[BASS]-1].value = addSemitones(middle_note, fancy_change[global_mode][(old_index + alt)%7], 0);
                            middle_note = bass_notes[num_notes[BASS]-1].value;
                        } 

                        bass_notes[num_notes[BASS]-1].value *= sounder;
                        chord_blocks[i].fancy++;
                    }
                }
                else
                    num_notes[BASS]--;                
            }

            if(chord_blocks[i].chord_data.index == chord_blocks[i-1].chord_data.index && chord_blocks[i].chord_data.type == chord_blocks[i-1].chord_data.type)
                bass_notes[num_notes[BASS]-1].keep_gap = (rand() % 5 == 0 || start_beat == global_meter-1);
            else if(start_beat == global_meter-1)
                bass_notes[num_notes[BASS]-1].keep_gap = 1;
        }

        newNotes * tenor_notes = (newNotes*) malloc(sizeof(newNotes));

        tenor_notes[0].value = chord_blocks[0].chord_data.notes[TENOR];
        tenor_notes[0].length = QUARTER;
        tenor_notes[0].start_measure = 1;
        tenor_notes[0].start_beat = 0;
        tenor_notes[0].keep_gap = 1;

        start_beat = 0;
        start_measure = 1;

        printf("Generating tenor notes...\n");

        for(int i = 1; i < num_chords; i++)
        {
            if(++start_beat == global_meter)
            {
                start_beat %= global_meter;
                start_measure++;
            }

            num_notes[TENOR]++;
            tenor_notes = (newNotes*) realloc(tenor_notes, num_notes[TENOR]*sizeof(newNotes));
            tenor_notes[num_notes[TENOR]-1].value = chord_blocks[i].chord_data.notes[TENOR];
            tenor_notes[num_notes[TENOR]-1].length = chord_blocks[i].num_beats;// - loss;
            tenor_notes[num_notes[TENOR]-1].start_beat = start_beat;
            tenor_notes[num_notes[TENOR]-1].start_measure = start_measure;
            tenor_notes[num_notes[TENOR]-1].keep_gap = 1;

            if(i >= num_chords - 3)
                continue;
            else if(rand() % 13 < 3)
            {
                num_notes[TENOR]++;
                tenor_notes = (newNotes*) realloc(tenor_notes, num_notes[TENOR]*sizeof(newNotes));
                int old_note = tenor_notes[num_notes[TENOR]-2].value;
                int x = rand() % NUM_VOICES; int middle_note = chord_blocks[i].chord_data.notes[x];
                int next_note = chord_blocks[i+1].chord_data.notes[TENOR];
                int keeper = 1;
                int sounder = (rand() % 10 > 2);
                if(middle_note == old_note)
                    keeper = (rand() % 10 > 2);
                
                if((mod_scale(middle_note) != FI && mod_scale(middle_note) != TI) || next_note == addSemitones(middle_note, 1, 0))
                {
                    float split = ((rand() % 8 < 3) + 2) * SIXTEENTH;
                    tenor_notes[num_notes[TENOR]-1].value = middle_note * sounder;
                    tenor_notes[num_notes[TENOR]-2].length = split;
                    tenor_notes[num_notes[TENOR]-1].length = QUARTER - split;
                    tenor_notes[num_notes[TENOR]-1].start_beat = start_beat;
                    tenor_notes[num_notes[TENOR]-1].start_measure = start_measure;
                    tenor_notes[num_notes[TENOR]-2].keep_gap = keeper;
                    tenor_notes[num_notes[TENOR]-1].keep_gap = (rand() % 5 > 1);

                    int old_index = getScaleIndex(old_note);
                    int alt = rand() % 7 - 1;

                    if(abs(alt) == 1 && chord_blocks[i].fancy < 2 && chord_blocks[i].chord_data.type == TONIC)
                    {
                        middle_note = scales[global_mode][(old_index + alt)%7];
                        if(old_index + alt < 0)
                            middle_note *= -1;
                        else if(old_index + alt >= 7)
                            middle_note += OCTAVE;

                        tenor_notes[num_notes[TENOR]-1].value = middle_note;

                        if(global_mode == MINOR && (mod_scale(middle_note) == TE || (mod_scale(middle_note) == LE && (mod_scale(chord_blocks[i+1].chord_data.root) == SO || mod_scale(next_note) == TI))))
                        {
                            tenor_notes[num_notes[TENOR]-1].value = addSemitones(middle_note, fancy_change[global_mode][(old_index + alt)%7], 0);
                            middle_note = tenor_notes[num_notes[TENOR]-1].value;
                        } 

                        tenor_notes[num_notes[TENOR]-1].value *= sounder;
                        chord_blocks[i].fancy++;
                    }
                }
                else
                    num_notes[TENOR]--;                
            }

            if(chord_blocks[i].chord_data.index == chord_blocks[i-1].chord_data.index && chord_blocks[i].chord_data.type == chord_blocks[i-1].chord_data.type)
                tenor_notes[num_notes[TENOR]-1].keep_gap = (rand() % 5 == 0 || start_beat == global_meter-1);
            else if(start_beat == global_meter-1)
                tenor_notes[num_notes[TENOR]-1].keep_gap = 1;   
        }

        newNotes * alto_notes = (newNotes*) malloc(sizeof(newNotes));

        alto_notes[0].value = chord_blocks[0].chord_data.notes[ALTO];
        alto_notes[0].length = QUARTER;
        alto_notes[0].start_measure = 1;
        alto_notes[0].start_beat = 0;
        alto_notes[0].keep_gap = 1;

        start_beat = 0;
        start_measure = 1;

        printf("Generating alto notes...\n");

        for(int i = 1; i < num_chords; i++)
        {
            if(++start_beat == global_meter)
            {
                start_beat %= global_meter;
                start_measure++;
            }

            num_notes[ALTO]++;
            alto_notes = (newNotes*) realloc(alto_notes, num_notes[ALTO]*sizeof(newNotes));
            alto_notes[num_notes[ALTO]-1].value = chord_blocks[i].chord_data.notes[ALTO];
            alto_notes[num_notes[ALTO]-1].length = chord_blocks[i].num_beats;// - loss;
            alto_notes[num_notes[ALTO]-1].start_beat = start_beat;
            alto_notes[num_notes[ALTO]-1].start_measure = start_measure;
            alto_notes[num_notes[ALTO]-1].keep_gap = 1;

            if(i >= num_chords - 3)
                continue;
            else if(rand() % 13 < 3)
            {
                num_notes[ALTO]++;
                alto_notes = (newNotes*) realloc(alto_notes, num_notes[ALTO]*sizeof(newNotes));
                int old_note = alto_notes[num_notes[ALTO]-2].value;
                int x = rand() % NUM_VOICES; int middle_note = chord_blocks[i].chord_data.notes[x];
                int next_note = chord_blocks[i+1].chord_data.notes[ALTO];
                int keeper = 1;
                int sounder = (rand() % 10 > 2);
                if(middle_note == old_note)
                    keeper = (rand() % 10 > 2);
                
                if((mod_scale(middle_note) != FI && mod_scale(middle_note) != TI) || next_note == addSemitones(middle_note, 1, 0))
                {
                    float split = ((rand() % 8 < 3) + 2) * SIXTEENTH;
                    alto_notes[num_notes[ALTO]-1].value = middle_note * sounder;
                    alto_notes[num_notes[ALTO]-2].length = split;
                    alto_notes[num_notes[ALTO]-1].length = QUARTER - split;
                    alto_notes[num_notes[ALTO]-1].start_beat = start_beat;
                    alto_notes[num_notes[ALTO]-1].start_measure = start_measure;
                    alto_notes[num_notes[ALTO]-2].keep_gap = keeper;
                    alto_notes[num_notes[ALTO]-1].keep_gap = (rand() % 5 > 1);

                    int old_index = getScaleIndex(old_note);
                    int alt = rand() % 7 - 1;

                    if(abs(alt) == 1 && chord_blocks[i].fancy < 2 && chord_blocks[i].chord_data.type == TONIC)
                    {
                        middle_note = scales[global_mode][(old_index + alt)%7];
                        if(old_index + alt < 0)
                            middle_note *= -1;
                        else if(old_index + alt >= 7)
                            middle_note += OCTAVE;

                        alto_notes[num_notes[ALTO]-1].value = middle_note;

                        if(global_mode == MINOR && (mod_scale(middle_note) == TE || (mod_scale(middle_note) == LE && (mod_scale(chord_blocks[i+1].chord_data.root) == SO || mod_scale(next_note) == TI))))
                        {
                            alto_notes[num_notes[ALTO]-1].value = addSemitones(middle_note, fancy_change[global_mode][(old_index + alt)%7], 0);
                            middle_note = alto_notes[num_notes[ALTO]-1].value;
                        } 

                        alto_notes[num_notes[ALTO]-1].value *= sounder;
                        chord_blocks[i].fancy++;
                    }
                }
                else
                    num_notes[ALTO]--;                
            }

            if(chord_blocks[i].chord_data.index == chord_blocks[i-1].chord_data.index && chord_blocks[i].chord_data.type == chord_blocks[i-1].chord_data.type)
                alto_notes[num_notes[ALTO]-1].keep_gap = (rand() % 5 == 0 || start_beat == global_meter-1);
            else if(start_beat == global_meter-1)
                alto_notes[num_notes[ALTO]-1].keep_gap = 1;        
        }

        newNotes * soprano_notes = (newNotes*) malloc(sizeof(newNotes));

        soprano_notes[0].value = chord_blocks[0].chord_data.notes[SOPRANO];
        soprano_notes[0].length = QUARTER;
        soprano_notes[0].start_measure = 0;
        soprano_notes[0].start_beat = 0;
        soprano_notes[0].keep_gap = 1;

        start_beat = 0;
        start_measure = 0;
        int repeated_chords = 0;

        printf("Generating soprano notes...\n");

        for(int i = 1; i < num_chords; i++)
        {
            int new_val = chord_blocks[i].chord_data.notes[SOPRANO];
            static int first_val = DO;
            if(chord_blocks[i].chord_data.root == chord_blocks[i-1].chord_data.root && chord_blocks[i].chord_data.mode == chord_blocks[i-1].chord_data.mode && start_beat != global_meter - 1)
                repeated_chords++;
            else
                repeated_chords = 0;
    
            if(++start_beat == global_meter)
            {
                start_beat %= global_meter;
                start_measure++;
                first_val = chord_blocks[i].chord_data.notes[SOPRANO];
            }

            static int keeper = 0;
            keeper++;
            keeper %= global_meter;

            num_notes[SOPRANO]++;
            soprano_notes = (newNotes*) realloc(soprano_notes, num_notes[SOPRANO]*sizeof(newNotes));
            soprano_notes[num_notes[SOPRANO]-1].value = new_val;
            soprano_notes[num_notes[SOPRANO]-1].length = chord_blocks[i].num_beats;
            soprano_notes[num_notes[SOPRANO]-1].keep_gap = (keeper == 0 || keeper == global_meter-2);// || new_val != first_val);//0;

            if(i >= num_chords-3)
            {
                soprano_notes[num_notes[SOPRANO]-1].length = chord_blocks[i].num_beats;
                soprano_notes[num_notes[SOPRANO]-1].keep_gap = 1;
                continue;
            }
            else if(chord_blocks[i].fancy == 0 && (start_measure % 4 != 0 || repeated_chords > 1 || global_meter == 4) && (soprano_notes[num_notes[SOPRANO]-2].length > SIXTEENTH || start_beat % 2 != 0))
            {
                if(i % global_meter != global_meter - 1)
                    soprano_notes[num_notes[SOPRANO]-1].keep_gap = (rand()%5 < 3);

                int old_num = num_notes[SOPRANO]++;
                int old_index = getScaleIndex(soprano_notes[num_notes[SOPRANO]-2].value);
                int extra_index = rand() % SCALE_SIZE + old_index - SCALE_SIZE*0.5;

                int not_forced = ((extra_index != getScaleIndex(soprano_notes[num_notes[SOPRANO]-2].value) || rand() % 12 < 2) && rand() % 5 < 2);
    
                if(not_forced == 0 && repeated_chords > 1 - (global_meter < 4))
                    extra_index = getScaleIndex(soprano_notes[num_notes[SOPRANO]-2].value) + 2*(rand()%2) - 1;

                if(not_forced == 1 || repeated_chords > 1 - (global_meter < 4))
                {
                    int old_note = chord_blocks[i].chord_data.notes[SOPRANO];
                    int next_note = chord_blocks[i+1].chord_data.notes[SOPRANO];
                    int middle_note;

                    if(abs(subtractScaleIndices(getScaleIndex(next_note), extra_index, 1)) <= 1 + (rand()%11 == 0) || abs(subtractScaleIndices(getScaleIndex(old_note), extra_index, 1)) <= 1 + (rand()%11 == 0))
                    {
                        if((chord_blocks[i].chord_data.mode != DOMINANT || (extra_index != 5 && extra_index != 2)) && (chord_blocks[i].chord_data.mode != SUBDOMINANT || (extra_index != 4 && extra_index != 1 && extra_index != 6)))
                        {
                            int grace_note = (rand() % 9 < 2 && not_forced == 1);
                            float grace_length = SIXTEENTH * grace_note;

                            if(abs(subtractScaleIndices(getScaleIndex(next_note), extra_index, 1)) > 1)
                                grace_length = 0;

                            soprano_notes = (newNotes*) realloc(soprano_notes, num_notes[SOPRANO]*sizeof(newNotes));
                            soprano_notes[num_notes[SOPRANO]-2].length = EIGHTH + grace_length;
                            soprano_notes[num_notes[SOPRANO]-1].length = EIGHTH - grace_length;
                            soprano_notes[num_notes[SOPRANO]-1].value = soprano_notes[num_notes[SOPRANO]-2].value;
                            soprano_notes[num_notes[SOPRANO]-1].keep_gap = 0;

                            if(extra_index%7 != getScaleIndex(old_note))
                            {
                                soprano_notes[num_notes[SOPRANO]-1].value = scales[global_mode][extra_index%7];
                                if(extra_index < 0)
                                    soprano_notes[num_notes[SOPRANO]-1].value *= -1;
                                else if(extra_index >= 7)
                                    soprano_notes[num_notes[SOPRANO]-1].value += OCTAVE;

                                if(mod_scale(soprano_notes[num_notes[SOPRANO]-1].value) == FA && ((chord_blocks[i].chord_data.index == VofV && chord_blocks[i].chord_data.type == SUBDOMINANT) || (mod_scale(chord_blocks[i+1].chord_data.notes[SOPRANO]) == SO && chord_blocks[i+1].chord_data.type == DOMINANT))) 
                                    soprano_notes[num_notes[SOPRANO]-1].value += getSign(soprano_notes[num_notes[SOPRANO]-1].value);
                                else if(mod_scale(soprano_notes[num_notes[SOPRANO]-1].value) == TE && chord_blocks[i].chord_data.type == DOMINANT && global_mode == MINOR)
                                {
                                    if(next_note == DO)
                                        middle_note = soprano_notes[num_notes[SOPRANO]-1].value = -TI;
                                    else if(mod_scale(next_note) == DO && next_note != -DO)
                                        middle_note = soprano_notes[num_notes[SOPRANO]-1].value = next_note - getSign(next_note);
                                    else
                                    {
                                        num_notes[SOPRANO]--; 
                                        soprano_notes[num_notes[SOPRANO]-1].length = chord_blocks[i].num_beats;
                                    }
                                } 
                            }

                            if(num_notes[SOPRANO] > old_num)
                            {
                                int temp_value = soprano_notes[num_notes[SOPRANO]-1].value;

                                if(get_distance(middle_note, old_note) > FI-DO)
                                    middle_note = soprano_notes[num_notes[SOPRANO]-1].value = addSemitones(soprano_notes[num_notes[SOPRANO]-1].value, OCTAVE, 1);
                                else if(get_distance(middle_note, old_note) > FI-DO)
                                {
                                    middle_note = soprano_notes[num_notes[SOPRANO]-1].value = soprano_notes[num_notes[SOPRANO]-1].value * -1;
                                }

                                if(abs(get_distance(old_note, middle_note)) > OCTAVE && abs(get_distance(old_note, temp_value)) < abs(get_distance(old_note, middle_note)) && abs(subtractScaleIndices(getScaleIndex(old_note), extra_index, 1)) < 1)
                                    middle_note = soprano_notes[num_notes[SOPRANO]-1].value = temp_value;
                            }
                        }

                        else
                            num_notes[SOPRANO]--;
                    }
                    else
                        num_notes[SOPRANO]--;
                }
                else
                    num_notes[SOPRANO]--; 

                if(num_notes[SOPRANO] > old_num)
                    soprano_notes[num_notes[SOPRANO]-2].keep_gap = (rand() % 13 < 3);
                else
                    soprano_notes[num_notes[SOPRANO]-1].length = chord_blocks[i].num_beats;

                if(start_beat == global_meter-1)
                    soprano_notes[num_notes[SOPRANO]-1].keep_gap = 1;
            }
        }

        for(int i = 0; i < num_chords; i += global_meter)
        {
            if(print_chords == 0)
                break;
            else
                printf("\n%d. ", i+1);
    
            for(int j = 0; j < NUM_VOICES; j++)
            {
                int k = 0;
                while(k < global_meter && i+k < num_chords)
                {
                    printf("\t%-2d ", chord_blocks[i+k].chord_data.notes[j]);

                    if(chord_blocks[i+k].chord_data.notes[j] < -DOH)
                    {
                        int l = 0;
                        while(chord_blocks[i+k].chord_data.notes[j] + 12*++l < -DOH);

                        printf(" -%-5s", solfege[-(chord_blocks[i+k].chord_data.notes[j] + 12*l)]);
                    }
                    else if(chord_blocks[i+k].chord_data.notes[j] < 0)
                        printf("-%-5s", solfege[-chord_blocks[i+k].chord_data.notes[j]]);
                    else if(chord_blocks[i+k].chord_data.notes[j] > DOH)
                    {
                        int l = 0;
                        while(chord_blocks[i+k].chord_data.notes[j] - 12*++l > DOH);

                        printf(" %-5s", solfege[chord_blocks[i+k].chord_data.notes[j]-12*l]);
                    }
                    else
                        printf(" %-5s", solfege[chord_blocks[i+k].chord_data.notes[j]]);

                    k++;
                }
                printf("\n");
            }
        }

        samples_per_beat = RATE * 60.0/((float)global_tempo);
        float frequencies[13];
               
        int tonic = getTonic(global_index, global_mode);
        float tuning_pitch = A4;
        if(tonic > D)
            tuning_pitch = A3;

        frequencies[0] = getFreqFromValue(tuning_pitch, tonic - A + 1);
        for(int i = 1; i < 13; i++)
            frequencies[i] = getFreqFromValue(frequencies[0], i+1);

        float lowest_frequency = frequencies[0];
        float highest_frequency = frequencies[0];

        breakpoint * soprano = (breakpoint*) malloc(num_notes[SOPRANO] * sizeof(breakpoint));
        breakpoint * alto = (breakpoint*) malloc(num_notes[ALTO] * sizeof(breakpoint));
        breakpoint * tenor = (breakpoint*) malloc(num_notes[TENOR] * sizeof(breakpoint));
        breakpoint * bass = (breakpoint*) malloc(num_notes[BASS] * sizeof(breakpoint));

        breakpoint * breakpoints[4] = {soprano, alto, tenor, bass};
        newNotes * voices[4] = {soprano_notes, alto_notes, tenor_notes, bass_notes};

        int num_samples = total_beats * samples_per_beat;
        
        printf("Generating breakpoints...\n");
        for(int j = 0; j < 4; j++)
        {
            if(abort_me == 1)
                break;

            for(int i = 0; i < num_notes[j]; i++)
            {
                static float old_freq[4] = {0.0, 0.0, 0.0, 0.0};
                if(voices[j][i].value == REST)
                    breakpoints[j][i].frequency = 0;
                else if(voices[j][i].value < 0)
                {
                    int low_val = -1*voices[j][i].value;
                    if(voices[j][i].value < -DOH)
                        breakpoints[j][i].frequency = 0;
                    else
                        breakpoints[j][i].frequency = (frequencies[low_val-1] * pow(0.5, j+1));
                }
                else if(voices[j][i].value > DOH)
                {
                    int high_val = voices[j][i].value;
                    int x = 0;
                    while(high_val > DOH)
                    {
                        x--;
                        high_val -= OCTAVE;
                        if(high_val > 48)
                            break;
                    }

                    if(high_val > 48)
                        breakpoints[j][i].frequency = 0;
                    else
                        breakpoints[j][i].frequency = (frequencies[high_val-1] * pow(0.5, j+x));
                }
                else
                    breakpoints[j][i].frequency = (frequencies[voices[j][i].value-1] * pow(0.5, j));

                breakpoints[j][i].num_frames = (int)(samples_per_beat * voices[j][i].length);
                
                if(i == 0)
                    breakpoints[j][i].start = 0;
                else
                {
                    breakpoints[j][i].start = breakpoints[j][i-1].start + breakpoints[j][i-1].num_frames;
                    if(i > 0 && (breakpoints[j][i].frequency != old_freq[j] || voices[j][i-1].keep_gap == 1))
                        breakpoints[j][i-1].gap = 250;
                }
                
                breakpoints[j][i].gap = 0;
                old_freq[j] = breakpoints[j][i].frequency;

                if(breakpoints[j][i].frequency < lowest_frequency && breakpoints[j][i].frequency > 1)
                    lowest_frequency = breakpoints[j][i].frequency;
                
                if(breakpoints[j][i].frequency > highest_frequency && breakpoints[j][i].frequency > 1)
                    highest_frequency = breakpoints[j][i].frequency;

                if(voices[j][i].start_beat == 0)
                    breakpoints[j][i].downbeat = 1;
                else
                    breakpoints[j][i].downbeat = 0;
            }
        }

        int noted = 0;
        lowest_frequency = A3;
        highest_frequency = A3;

        for(int j = 0; j <= BASS ; j++)
        {
            for(int i = 0; i < num_notes[j]; i++)
            {
                float old_freq = breakpoints[j][i].frequency;
                
                while(limitRange(breakpoints[j][i].frequency, j) != 0)
                    breakpoints[j][i].frequency *= pow(2, limitRange(breakpoints[j][i].frequency, j));
                
                if(breakpoints[j][i].frequency < lowest_frequency && breakpoints[j][i].frequency > 1)
                    lowest_frequency = breakpoints[j][i].frequency;
            
                if(breakpoints[j][i].frequency > highest_frequency && breakpoints[j][i].frequency > 1)
                    highest_frequency = breakpoints[j][i].frequency;

                if(old_freq == breakpoints[j][i].frequency)
                    continue;
                else if(noted++ == 0)
                    printf("\nCompressing range by transposing individual notes...\n");
            }
        }


        FILE * outfile = fopen(argv[ARG_OUTFILE], "wb");

        int METRO_SIZE = 2*global_meter;
        float metro1 = getFreqFromValue(4*A4, MI);
        float metro2 = getFreqFromValue(2*A4, TE);

        breakpoint * metronome = (breakpoint*) malloc(METRO_SIZE * sizeof(breakpoint));
        for(int i = 0; i < METRO_SIZE; i++)
        {
            if(i % global_meter == 0)
                metronome[i].frequency = metro1;
            else
                metronome[i].frequency = metro2;

            metronome[i].num_frames = samples_per_beat;
            metronome[i].gap = (int)(0.9 * samples_per_beat);
            metronome[i].start = i * samples_per_beat;
        }

        breakpoint * temp[3];
        int temp_notes[4] = {METRO_SIZE, 0, 0, 0};

        printf("\nWriting count-in beats to file... \n");

        for(int i = 0; i < METRO_SIZE * samples_per_beat; i++)
        {
            float intro_sample = combineSamples(i, metronome, temp[0], temp[1], temp[2], temp_notes);
            if(fwrite(&intro_sample, sizeof(float), 1, outfile) != 1 || abort_me != 0)
            {
                if(abort_me == 0)
                    printf("Error writing data to file!\n");

                free(metronome);
                free(soprano);
                free(alto);
                free(tenor);
                free(bass);
                free(soprano_notes);
                free(alto_notes);
                free(tenor_notes);
                free(bass_notes);
                free(chord_blocks);
                fclose(outfile);
                return 1;
            }
        }

        free(metronome);
        printf("Saving song to file...\n");

        for(int i = 0; i < num_samples; i++)
        {
            float full_sample = combineSamples(i, soprano, alto, tenor, bass, num_notes);
            
            if(fwrite(&full_sample, sizeof(float), 1, outfile) != 1 || abort_me != 0)
            {
                if(abort_me == 0)
                    printf("Error writing data to file!\n");

                free(soprano);
                free(alto);
                free(tenor);
                free(bass);
                free(soprano_notes);
                free(alto_notes);
                free(tenor_notes);
                free(bass_notes);
                free(chord_blocks);
                fclose(outfile);
                return 1;
            }
        }

        free(soprano_notes); 
        free(alto_notes); 
        free(tenor_notes);
        free(bass_notes);
        free(chord_blocks);
        free(soprano);
        free(alto);
        free(tenor);
        free(bass);

        int order = byte_order();
        printf("\n%d %s samples written to %s!\nSample rate: %d\nTotal length: %d seconds!\nTotal measures: %d\nLowest frequency: %f\nHighest frequency: %f\nGoodbye!\n", num_samples, endianness[order], argv[ARG_OUTFILE], (int)RATE, (num_samples/22050), start_measure + 2, lowest_frequency, highest_frequency);

        break;
    }

    return 0;
}