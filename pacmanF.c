#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 20
#define PACMAN 'C'
#define WALL '#'
#define FOOD '.'
#define EMPTY ' '
#define DEMON 'X'

int pacman_x, pacman_y;
char board[HEIGHT][WIDTH];
int score = 0;
int food = 0;
int res = 0;

// Function prototypes
void initialize();
void draw_game(GtkWidget *widget, cairo_t *cr, gpointer data);
void move_pacman(int dx, int dy);
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);
void show_game(GtkWidget *widget, gpointer data);
void register_user(GtkWidget *widget, gpointer data);
void login_user(GtkWidget *widget, gpointer data);
void add_user_to_file(const char *username, const char *password);
int validate_user(const char *username, const char *password);

// Initializes the game board
void initialize() {
    // Initialize walls
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || j == WIDTH - 1 || j == 0 || i == HEIGHT - 1) {
                board[i][j] = WALL;
            } else {
                board[i][j] = EMPTY;
            }
        }
    }

    // Place random walls inside the game
    int count = 50;
    while (count--) {
        int i = rand() % HEIGHT;
        int j = rand() % WIDTH;
        if (board[i][j] != WALL) {
            board[i][j] = WALL;
        }
    }

    // Place demons
    count = 10;
    while (count--) {
        int i = rand() % HEIGHT;
        int j = rand() % WIDTH;
        if (board[i][j] != WALL) {
            board[i][j] = DEMON;
        }
    }

    // Place food
    for (int i = 1; i < HEIGHT - 1; i++) {
        for (int j = 1; j < WIDTH - 1; j++) {
            if (i % 2 == 0 && j % 2 == 0 && board[i][j] != WALL && board[i][j] != DEMON) {
                board[i][j] = FOOD;
                food++;
            }
        }
    }

    // Set Pacman start position
    pacman_x = WIDTH / 2;
    pacman_y = HEIGHT / 2;
    board[pacman_y][pacman_x] = PACMAN;
}

// GTK Drawing function for the game board
void draw_game(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int block_size = 20;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == WALL) {
                cairo_set_source_rgb(cr, 0, 0, 0); // Black for walls
            } else if (board[i][j] == PACMAN) {
                cairo_set_source_rgb(cr, 1, 1, 0); // Yellow for Pacman
            } else if (board[i][j] == FOOD) {
                cairo_set_source_rgb(cr, 0, 1, 0); // Green for food
            } else if (board[i][j] == DEMON) {
                cairo_set_source_rgb(cr, 1, 0, 0); // Red for demon
            } else {
                cairo_set_source_rgb(cr, 1, 1, 1); // White for empty spaces
            }
            cairo_rectangle(cr, j * block_size, i * block_size, block_size, block_size);
            cairo_fill(cr);
        }
    }
}

// Function to move Pacman
void move_pacman(int dx, int dy) {
    int new_x = pacman_x + dx;
    int new_y = pacman_y + dy;

    if (board[new_y][new_x] != WALL) {
        if (board[new_y][new_x] == FOOD) {
            score++;
            food--;
            if (food == 0) {
                res = 1;
                return;
            }
        } else if (board[new_y][new_x] == DEMON) {
            res = 2;
            return;
        }

        board[pacman_y][pacman_x] = EMPTY;
        pacman_x = new_x;
        pacman_y = new_y;
        board[pacman_y][pacman_x] = PACMAN;
    }
}

// Keyboard event handler
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    switch (event->keyval) {
        case GDK_KEY_w: move_pacman(0, -1); break;
        case GDK_KEY_s: move_pacman(0, 1); break;
        case GDK_KEY_a: move_pacman(-1, 0); break;
        case GDK_KEY_d: move_pacman(1, 0); break;
    }
    gtk_widget_queue_draw(widget); // Redraw the game
    return TRUE;
}

// Show the game window
void show_game(GtkWidget *widget, gpointer data) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pacman Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_game), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), NULL);

    initialize(); // Initialize the game
    gtk_widget_show_all(window);
}

// User Registration
void register_user(GtkWidget *widget, gpointer data) {
    const char *username = gtk_entry_get_text(GTK_ENTRY(((GtkWidget **)data)[0]));
    const char *password = gtk_entry_get_text(GTK_ENTRY(((GtkWidget **)data)[1]));

    add_user_to_file(username, password);
    gtk_widget_destroy((GtkWidget *)data); // Close window after registration
}

// User Login
void login_user(GtkWidget *widget, gpointer data) {
    const char *username = gtk_entry_get_text(GTK_ENTRY(((GtkWidget **)data)[0]));
    const char *password = gtk_entry_get_text(GTK_ENTRY(((GtkWidget **)data)[1]));

    if (validate_user(username, password)) {
        show_game(widget, data); // Start the game if login successful
    } else {
        g_print("Invalid login!\n");
    }
}

// Add user to file
void add_user_to_file(const char *username, const char *password) {
    FILE *file = fopen("users.txt", "a");
    if (file) {
        fprintf(file, "%s %s\n", username, password);
        fclose(file);
    }
}

// Validate user from file
int validate_user(const char *username, const char *password) {
    FILE *file = fopen("users.txt", "r");
    if (!file) return 0;

    char stored_user[50], stored_pass[50];
    while (fscanf(file, "%s %s", stored_user, stored_pass) != EOF) {
        if (strcmp(username, stored_user) == 0 && strcmp(password, stored_pass) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pacman Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create entry widgets for username and password
    GtkWidget *username_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    // Create buttons
    GtkWidget *login_button = gtk_button_new_with_label("Login");
    GtkWidget *register_button = gtk_button_new_with_label("Register");

    // Place widgets in the grid
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Username:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Password:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), login_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), register_button, 2, 2, 1, 1);

    // Connect button signals
    GtkWidget *user_widgets[] = {username_entry, password_entry};
    g_signal_connect(G_OBJECT(login_button), "clicked", G_CALLBACK(login_user), user_widgets);
    g_signal_connect(G_OBJECT(register_button), "clicked", G_CALLBACK(register_user), user_widgets);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;u
}
