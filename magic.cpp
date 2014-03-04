#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "magic.h"
#include "main.h"

void write_magic(std::string filename, rows_t& rows, channels_t& channels)
{
    // transform matrices
    // xy flip [-1, 0, x, 0, -1, y]
    // x  flip [ 1, 0, x, 0, -1, y]
    // y  flip [-1, 0, x, 0,  1, y]
    // no flip [ 1, 0, x, 0,  1, y]

    filename.append(".mag");

    FILE *fp = fopen(filename.c_str(), "w");

    fprintf(fp, "magic\n");
    fprintf(fp, "tech scmos\n");

    //
    // Place all the cells
    //

    for (auto &row : rows) {
        for (auto &cell : row) {

            if (cell->feed_through) {
                fprintf(fp, "use feed_through feed_though_%d\n", cell->number);
            } else {
                fprintf(fp, "use cell cell_%d\n", cell->number);
            }

            if (cell->flip_x && cell->flip_y) {
                fprintf(fp, "transform -1  0 %d 0 -1 %d\n", cell->position.x + 6, cell->position.y + 6);
            } else if (cell->flip_x) {
                fprintf(fp, "transform  1  0 %d 0 -1 %d\n", cell->position.x + 0, cell->position.y + 6);
            } else if (cell->flip_y) {
                fprintf(fp, "transform -1  0 %d 0  1 %d\n", cell->position.x + 6, cell->position.y + 0);
            } else {
                fprintf(fp, "transform  1  0 %d 0  1 %d\n", cell->position.x + 0, cell->position.y + 0);
            }

            if (cell->feed_through) {
                fprintf(fp, "box 0 0 3 6\n");
            } else {
                fprintf(fp, "box 0 0 6 6\n");
            }
        }
    }

    //
    // Place all the horizontal routes
    //

    fprintf(fp, "<< metal1 >>\n");
    for (auto &row : rows) {
        for (auto &cell : row) {
            int num_terms = cell->feed_through ? 2 : 4;
            for (int term=0; term<num_terms; term++) {

                term_t *src_term = &cell->terms[term];
                term_t *dst_term = cell->terms[term].dest_term;

                if (src_term->dest_cell == nullptr) continue;
                if (src_term->track == -1) continue;

                point_t p1 = src_term->position();
                point_t p2 = dst_term->position();

                if (src_term->on_top()) {
                    p1.y += 1 + CELL_SPACING + src_term->track * (TRACK_WIDTH + TRACK_SPACING);
                } else {
                    p1.y -= 1 + CELL_SPACING + src_term->track * (TRACK_WIDTH + TRACK_SPACING);
                }

                p2.y = p1.y;
                int x1 = std::min(p1.x, p2.x);
                int x2 = std::max(p1.x, p2.x)+1;
                int y1 = p1.y;
                int y2 = p1.y + 1;
                fprintf(fp, "rect %d %d %d %d\n", x1, y1, x2, y2);
            }
        }
    }

    //
    // Place all the vertical routes
    //

    fprintf(fp, "<< metal2 >>\n");
    for (auto &row : rows) {
        for (auto &cell : row) {
            int num_terms = cell->feed_through ? 2 : 4;
            for (int term=0; term<num_terms; term++) {

                term_t *src_term = &cell->terms[term];
                term_t *dst_term = src_term->dest_term;

                if (dst_term == nullptr) continue;
                if (src_term->track == -1) continue;

                point_t p1, p2;
                int x1, y1, x2, y2;

                // source terminal
                p1 = src_term->position();
                p2 = p1;
                if (src_term->on_top()) {
                    p2.y += src_term->track * (TRACK_WIDTH + TRACK_SPACING);
                } else {
                    p2.y -= src_term->track * (TRACK_WIDTH + TRACK_SPACING);
                }
                x1 = std::min(p1.x, p2.x);
                x2 = std::max(p1.x, p2.x);
                y1 = std::min(p1.y, p2.y);
                y2 = std::max(p1.y, p2.y);
                fprintf(fp, "rect %d %d %d %d\n", x1, y1, x2+TRACK_WIDTH, y2+TRACK_WIDTH);

                // destination terminal
                p1 = dst_term->position();
                p2 = p1;
                if (dst_term->on_top()) {
                    p2.y += dst_term->track * (TRACK_WIDTH + TRACK_SPACING);
                } else {
                    p2.y -= dst_term->track * (TRACK_WIDTH + TRACK_SPACING);
                }
                x1 = std::min(p1.x, p2.x);
                x2 = std::max(p1.x, p2.x);
                y1 = std::min(p1.y, p2.y);
                y2 = std::max(p1.y, p2.y);
                fprintf(fp, "rect %d %d %d %d\n", x1, y1, x2+TRACK_WIDTH, y2+TRACK_WIDTH);
            }
        }
    }


    //
    // Place all the vias
    //

    fprintf(fp, "<< m2contact >>\n");
    for (auto &row : rows) {
        for (auto &cell : row) {
            int num_terms = cell->feed_through ? 2 : 4;
            for (int term=0; term<num_terms; term++) {

                term_t *src_term = &cell->terms[term];
                term_t *dst_term = src_term->dest_term;

                if (dst_term == nullptr) continue;
                if (src_term->track == -1) continue;

                point_t p1 = src_term->position();
                point_t p2 = dst_term->position();

                if (src_term->on_top()) {
                    p1.y += 1 + CELL_SPACING + src_term->track * (TRACK_WIDTH + TRACK_SPACING);
                } else {
                    p1.y -= 1 + CELL_SPACING + src_term->track * (TRACK_WIDTH + TRACK_SPACING);
                }
                p2.y = p1.y;

                fprintf(fp, "rect %d %d %d %d\n", p1.x, p1.y, p1.x+1, p1.y+1);
                fprintf(fp, "rect %d %d %d %d\n", p2.x, p2.y, p2.x+1, p2.y+1);
            }
        }
    }

    fprintf(fp, "<< end >>\n");
    fclose(fp);
}
