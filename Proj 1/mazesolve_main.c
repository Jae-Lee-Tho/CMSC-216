#include "mazesolve.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *filename = NULL;
    
    // Check command line usage
    if (argc == 2) {
        // Form 1: ./mazesolve_main <mazefile>
        filename = argv[1];
    } else if (argc == 4) {
        // Form 2: ./mazesolve_main -log <N> <mazefile>
        if (strcmp(argv[1], "-log") == 0) {
            LOG_LEVEL = atoi(argv[2]);
            filename = argv[3];
        } else {
            // Print usage information and return error if arguments are invalid
            fprintf(stderr, "Usage: %s [-log <level>] <maze-file>\n", argv[0]);
            return 1;
        }
    } else {
         // Print usage information if argument count is incorrect
        fprintf(stderr, "Usage: %s [-log <level>] <maze-file>\n", argv[0]);
        return 1;
    }
    
    // Attempt to load the maze from the file
    maze_t *maze = maze_from_file(filename);
    if (maze == NULL) {
        printf("Could not load maze file. Exiting with error code 1\n");
        return 1;
    }
    
    // Print the unsolved maze tiles
    maze_print_tiles(maze);
    
    // Solve the maze using BFS
    maze_bfs_iterate(maze);
    
    // Set the solution on the maze.
    // If a solution is found, print "SOLUTION:" then the solved maze and the path.
    if (maze_set_solution(maze)) {
        printf("SOLUTION:\n");
        maze_print_tiles(maze);
        // Print the solution path in verbose format.
        tile_print_path(&(maze->tiles[maze->end_row][maze->end_col]), PATH_FORMAT_VERBOSE);
    } else {
        printf("NO SOLUTION FOUND\n");
    }
    
    maze_free(maze);
    return 0;
}

