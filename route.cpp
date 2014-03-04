#include <stdio.h>
#include <vector>
#include "route.h"
#include "main.h"

channels_t route(rows_t& rows)
{
    /*
       ============== CHANNEL 5
       [] [] [] [] [] ROW 4
       ============== CHANNEL 4
       [] [] [] [] [] ROW 3
       ============== CHANNEL 3
       [] [] [] [] [] ROW 2
       ============== CHANNEL 2
       [] [] [] [] [] ROW 1
       ============== CHANNEL 1
       [] [] [] [] [] ROW 0
       ============== CHANNEL 0
    */

    /*
       ...
       -------------- CHANNEL 2 TRACK 0
       -------------- CHANNEL 2 TRACK 1
       -------------- CHANNEL 2 TRACK 2
       [] [] [] [] [] ROW 1
       -------------- CHANNEL 1 TRACK 0
       -------------- CHANNEL 1 TRACK 1
       -------------- CHANNEL 1 TRACK 2
       [] [] [] [] [] ROW 0
       -------------- CHANNEL 0 TRACK 0
       -------------- CHANNEL 0 TRACK 1
       -------------- CHANNEL 0 TRACK 2
    */


    // list of terminals in each channel
    channels_t channels(rows.size()+1);

    // go through the terminals of each cell in each row and add the terminals
    // to the correct channels
    for (unsigned int row_idx=0; row_idx<rows.size(); row_idx++) {
        for (auto &cell : rows[row_idx]) {
            int num_terms = cell->feed_through ? 2 : 4;
            for (int term=0; term<num_terms; term++) {
                if (cell->term[term].on_top()) {
                    channels[row_idx + 1].terms.push_back(&cell->term[term]);
                } else {
                    channels[row_idx].terms.push_back(&cell->term[term]);
                }
            }
        }
    }


    for (auto &channel : channels) {
        for (auto &term : channel.terms) {
            if (term->dest_cell == nullptr) continue;
            if (term->track >= 0) continue;
            term->track = channel.tracks.size();
            term->dest_cell->term[term->dest_term].track = channel.tracks.size();
            std::vector<term_t*> track;
            track.push_back(term);
            track.push_back(&term->dest_cell->term[term->dest_term]);
            channel.tracks.push_back(track);
        }
    }

    return channels;
}
