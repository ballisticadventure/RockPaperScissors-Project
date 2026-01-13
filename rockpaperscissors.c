#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* GLOBALS */
GtkWidget *login_window, *mode_window, *game_window, *final_window;
char player_name[100] = "";
int rounds_to_play = 3;    // best-of value
int player_score = 0;
int cpu_score = 0;
int draw_count = 0;
int current_round = 1;

/* Forward declarations */
void open_mode_window();
void open_game_window();
void open_final_window();

/* Utility: random CPU choice */
const char* cpu_pick() {
    const char *choices[3] = {"Rock", "Paper", "Scissors"};
    return choices[rand() % 3];
}

/* Utility: get winner text */
const char* winner(const char *p, const char *c) {
    if (strcmp(p, c) == 0) return "Draw";

    if ((strcmp(p, "Rock") == 0 && strcmp(c, "Scissors") == 0) ||
        (strcmp(p, "Paper") == 0 && strcmp(c, "Rock") == 0) ||
        (strcmp(p, "Scissors") == 0 && strcmp(c, "Paper") == 0))
        return "Player";

    return "CPU";
}

/* make a window with background */
GtkWidget* themed_window(const char *title, GtkWidget **overlay_out) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), title);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(win), overlay);

    /* Load background */
    GError *err = NULL;
    GdkPixbuf *bg_pixbuf = gdk_pixbuf_new_from_file("assets/bg.png", &err);
    if (!bg_pixbuf) {
        g_printerr("Error loading bg.png: %s\n", err->message);
        exit(1);
    }

    /* Match window size to background */
    int w = gdk_pixbuf_get_width(bg_pixbuf);
    int h = gdk_pixbuf_get_height(bg_pixbuf);
    gtk_window_set_default_size(GTK_WINDOW(win), w, h);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);

    GtkWidget *bg = gtk_image_new_from_pixbuf(bg_pixbuf);
    gtk_widget_set_halign(bg, GTK_ALIGN_FILL);
    gtk_widget_set_valign(bg, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(bg, TRUE);
    gtk_widget_set_vexpand(bg, TRUE);

    gtk_container_add(GTK_CONTAINER(overlay), bg);

    *overlay_out = overlay;
    return win;
}


/* ---------------- LOGIN WINDOW ---------------- */
void on_login_clicked(GtkWidget *btn, gpointer entry_ptr) {
    GtkWidget *entry = GTK_WIDGET(entry_ptr);
    strcpy(player_name, gtk_entry_get_text(GTK_ENTRY(entry)));

    if (strlen(player_name) == 0) return;

    gtk_widget_hide(login_window);
    open_mode_window();
}

void open_login_window() {
    GtkWidget *overlay;
    login_window = themed_window("Login", &overlay);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    GtkWidget *title = gtk_label_new("Enter Your Name");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 5);

    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Your Name");
    gtk_widget_set_size_request(entry, 250, 40);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 10);

    GtkWidget *btn = gtk_button_new_with_label("Continue");
    gtk_widget_set_size_request(btn, 200, 50);
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 10);

    g_signal_connect(btn, "clicked", G_CALLBACK(on_login_clicked), entry);

    gtk_widget_show_all(login_window);
}

/* ---------------- MODE WINDOW ---------------- */
void mode_choice_fixed(GtkWidget *btn, gpointer data) {
    rounds_to_play = GPOINTER_TO_INT(data);
    gtk_widget_hide(mode_window);
    open_game_window();
}

void custom_mode_entered(GtkWidget *btn, gpointer entry_ptr) {
    GtkWidget *entry = GTK_WIDGET(entry_ptr);
    int n = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
    if (n >= 1) {
        rounds_to_play = n;
        gtk_widget_hide(mode_window);
        open_game_window();
    }
}

void open_mode_window() {
    GtkWidget *overlay;
    mode_window = themed_window("Choose Mode", &overlay);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    char greeting[150];
    sprintf(greeting, "Hi, %s!\nChoose a game mode:", player_name);

    GtkWidget *label = gtk_label_new(greeting);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 10);

    GtkWidget *btn3 = gtk_button_new_with_label("Best of 3");
    GtkWidget *btn5 = gtk_button_new_with_label("Best of 5");
    GtkWidget *custom_entry = gtk_entry_new();
    GtkWidget *btn_custom = gtk_button_new_with_label("Start Custom Match");

    gtk_entry_set_placeholder_text(GTK_ENTRY(custom_entry), "Enter number");

    gtk_box_pack_start(GTK_BOX(box), btn3, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn5, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), custom_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_custom, FALSE, FALSE, 5);

    g_signal_connect(btn3, "clicked", G_CALLBACK(mode_choice_fixed), GINT_TO_POINTER(3));
    g_signal_connect(btn5, "clicked", G_CALLBACK(mode_choice_fixed), GINT_TO_POINTER(5));
    g_signal_connect(btn_custom, "clicked", G_CALLBACK(custom_mode_entered), custom_entry);

    gtk_widget_show_all(mode_window);
}

/* ---------------- GAME WINDOW ---------------- */
GtkWidget *score_label;
GtkWidget *round_label;
GtkWidget *result_label;

void update_game_labels(const char *player_choice, const char *cpu_choice, const char *win) {
    char score_text[200];
    sprintf(score_text, "Score: %s %d  -  CPU %d", player_name, player_score, cpu_score);
    gtk_label_set_text(GTK_LABEL(score_label), score_text);

    char round_text[100];
    sprintf(round_text, "Round %d / %d", current_round, rounds_to_play);
    gtk_label_set_text(GTK_LABEL(round_label), round_text);

    char res[200];
    sprintf(res, "You: %s   |   CPU: %s\nWinner: %s", player_choice, cpu_choice, win);
    gtk_label_set_text(GTK_LABEL(result_label), res);
}

void on_player_choice(GtkWidget *btn, gpointer data) {
    const char *player_choice = data;
    const char *cpu_choice = cpu_pick();
    const char *win = winner(player_choice, cpu_choice);

    if (strcmp(win, "Player") == 0) player_score++;
    else if (strcmp(win, "CPU") == 0) cpu_score++;
    else if (strcmp(win, "Draw") == 0) {
        draw_count++;
        // Don't increment round on draw, allow retry
        update_game_labels(player_choice, cpu_choice, win);
        return;
    }

    update_game_labels(player_choice, cpu_choice, win);

    current_round++;
    if (current_round > rounds_to_play) {
        gtk_widget_hide(game_window);
        open_final_window();
    }
}

void open_game_window() {
    player_score = 0;
    cpu_score = 0;
    draw_count = 0;
    current_round = 1;

    GtkWidget *overlay;
    game_window = themed_window("RPS Game", &overlay);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    score_label = gtk_label_new("");
    round_label = gtk_label_new("");
    result_label = gtk_label_new("Make your choice!");

    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    GtkWidget *btn_r = gtk_button_new_with_label("Rock");
    GtkWidget *btn_p = gtk_button_new_with_label("Paper");
    GtkWidget *btn_s = gtk_button_new_with_label("Scissors");

    gtk_box_pack_start(GTK_BOX(box), score_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), round_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), result_label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(box), row, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(row), btn_r, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(row), btn_p, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(row), btn_s, FALSE, FALSE, 5);

    g_signal_connect(btn_r, "clicked", G_CALLBACK(on_player_choice), "Rock");
    g_signal_connect(btn_p, "clicked", G_CALLBACK(on_player_choice), "Paper");
    g_signal_connect(btn_s, "clicked", G_CALLBACK(on_player_choice), "Scissors");

    update_game_labels("", "", "Start!");

    gtk_widget_show_all(game_window);
}

/* ---------------- FINAL RESULT WINDOW ---------------- */
void restart_game(GtkWidget *btn, gpointer data) {
    gtk_widget_hide(final_window);
    open_mode_window();
}

void open_final_window() {
    GtkWidget *overlay;
    final_window = themed_window("Final Result", &overlay);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    char text[300];
    const char *final_winner = (player_score > cpu_score) ? player_name :
                               (cpu_score > player_score) ? "CPU" : "No one";

    sprintf(text, "Final Winner:\n%s\n\nFinal Score\n%s %d  -  CPU %d\n\nDraws: %d",
            final_winner, player_name, player_score, cpu_score, draw_count);

    GtkWidget *label = gtk_label_new(text);
    GtkWidget *btn = gtk_button_new_with_label("Play Again");

    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 10);

    g_signal_connect(btn, "clicked", G_CALLBACK(restart_game), NULL);

    gtk_widget_show_all(final_window);
}

/* ---------------- MAIN ---------------- */
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL));

    open_login_window();

    gtk_main();
    return 0;
}