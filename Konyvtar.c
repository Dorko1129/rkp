#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

long long total_file_size = 0;
long long total_dir_size = 0;
int total_file_count = 0;
int total_dir_count = 0;

typedef struct InodeNode {
    ino_t inode;
    struct InodeNode *left;
    struct InodeNode *right;
} InodeNode;

InodeNode *root = NULL;

int is_seen_and_add(InodeNode **node, ino_t ino) {
    if (*node == NULL) {
        *node = (InodeNode *)malloc(sizeof(InodeNode));
        (*node)->inode = ino;
        (*node)->left = NULL;
        (*node)->right = NULL;
        return 0;
    }
    
    if (ino < (*node)->inode) {
        return is_seen_and_add(&((*node)->left), ino);
    } else if (ino > (*node)->inode) {
        return is_seen_and_add(&((*node)->right), ino);
    }
    
    return 1; 
}

void free_tree(InodeNode *node) {
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

void process_directory(const char *path, int depth) {
    DIR *dir = opendir(path);
    if (dir == NULL) return;

    struct dirent *entry;
    struct stat st;
    char full_path[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (lstat(full_path, &st) == 0) {
            if (is_seen_and_add(&root, st.st_ino)) {
                continue;
            }

            for (int i = 0; i < depth; i++) printf("  |--");

            if (S_ISDIR(st.st_mode)) {
                printf(" [DIR]  %s (%lld bytes)\n", entry->d_name, (long long)st.st_size);
                total_dir_count++;
                total_dir_size += st.st_size;
                process_directory(full_path, depth + 1);
            } else if (S_ISREG(st.st_mode)) {
                printf(" [FILE] %s (%lld bytes)\n", entry->d_name, (long long)st.st_size);
                total_file_count++;
                total_file_size += st.st_size;
            }
        }
    }
    closedir(dir);
}

int main() {
    printf("Directory tree:\n\n");
    process_directory(".", 0);

    printf("\n=========================================\n");
    printf("STATISTICS (Hardlinks ignored):\n");
    printf("=========================================\n");
    printf("Files count:       %d\n", total_file_count);
    printf("Files total size:  %lld bytes\n", total_file_size);
    printf("Dirs count:        %d\n", total_dir_count);
    printf("Dirs total size:   %lld bytes\n", total_dir_size);
    printf("=========================================\n");

    free_tree(root);
    return 0;
}
