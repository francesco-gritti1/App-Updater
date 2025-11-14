

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


#include <filesystem>
#include <iostream>
#include <libconfig.h++>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <array>
#include <format>


using std::cout, std::format;

#include "utils.h"
#include "json.h"
using json = nlohmann::json;




#define PKG_FILENAME  "packages.json"
#define PKG_DIRECTORY "backup/"
#define TRACKED_PKG_FILEPATH  PKG_DIRECTORY"tracked_packages.json"

namespace fs = std::filesystem;


static GtkListBox* list_box;
static std::vector <std::string> pkg_names;


void add_packages (GtkListBox* listbox);

void destroy_child(GtkWidget *widget, gpointer data) 
{
    gtk_widget_destroy(widget);
}
void clear_container(GtkContainer *container) 
{
    gtk_container_foreach(container, (GtkCallback)destroy_child, NULL);
}





#ifdef __cplusplus
extern "C" {
#endif

void refresh_button_clicked (GtkButton* button, gpointer data);
void properties_button_clicked (GtkButton* button, gpointer data);
void update_button_clicked (GtkButton* button, gpointer data);

#ifdef __cplusplus
}
#endif



void refresh_button_clicked (GtkButton* button, gpointer data)
{
  printf ("Refresh\n");

  check_packages ();
  clear_container (GTK_CONTAINER(list_box));
  add_packages (list_box);
}

void properties_button_clicked (GtkButton* button, gpointer data)
{
  printf ("Settings\n");
}

void update_button_clicked (GtkButton* button, gpointer data)
{   
    int i=(int)(intptr_t)data;
    cout << format("Button clicked for pkg {}\n", pkg_names[i]);

    update_pkg_named (pkg_names[i]);
    check_packages ();
    clear_container (GTK_CONTAINER(list_box));
    add_packages (list_box);
}





void add_packages (GtkListBox* listbox)
{

    std::ifstream tracked_file(TRACKED_PKG_FILEPATH);
    json json_tracked = json::parse(tracked_file);

    pkg_names.clear ();

    int i=0;
    for (auto& package: json_tracked["packages"])
    {   
        std::string version = package["installed_version"];
        std::string name_str = package["name"];
        pkg_names.push_back (name_str);

        GtkWidget *name, *grid, *version_label, *update_label, *button;
        GtkBuilder* builder = gtk_builder_new();
        GError *error = NULL;
        if (!gtk_builder_add_from_file(builder, "graphics/Package-Row.ui", &error)) {
            g_printerr("Errore caricamento UI: %s\n", error->message);
            g_error_free(error);
            continue;
        }
        gtk_builder_connect_signals(builder, NULL);

        name = GTK_WIDGET (gtk_builder_get_object(builder, "name"));
        grid = GTK_WIDGET (gtk_builder_get_object(builder, "grid"));
        version_label = GTK_WIDGET (gtk_builder_get_object(builder, "version_label"));
        update_label = GTK_WIDGET (gtk_builder_get_object(builder, "update_label"));
        button = GTK_WIDGET (gtk_builder_get_object(builder, "button"));

        g_signal_connect (button, "clicked", G_CALLBACK(update_button_clicked), (gpointer)(intptr_t)(i));
                

        gtk_label_set_text (GTK_LABEL (version_label), version.c_str());
        gtk_label_set_text (GTK_LABEL (name), name_str.c_str());

        if (package["update"] == true)
        {
            gtk_widget_show (button);
            gtk_label_set_text (GTK_LABEL (update_label), "YES");
        }
        else if (package["installed"] == false)
        {
            gtk_widget_show (button);
            gtk_button_set_label (GTK_BUTTON(button), "Installl");
        }
        else 
        {
            gtk_widget_hide (button);
            gtk_label_set_text (GTK_LABEL (update_label), "No");
        }


        gtk_widget_show (GTK_WIDGET (grid));
        gtk_container_add (GTK_CONTAINER (listbox), GTK_WIDGET (grid));

        g_object_unref(builder);
        i++;
    }
}



int main (int argc, char *argv[])
{

    check_packages ();


    GtkWindow* main_window;
    GtkBuilder *builder; 

	gtk_init(&argc, &argv);
 
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "graphics/App-Updater.ui", NULL);
	gtk_builder_connect_signals(builder, NULL);
 
	main_window = GTK_WINDOW (gtk_builder_get_object(builder, "main_window"));
    list_box = GTK_LIST_BOX (gtk_builder_get_object(builder, "list_box"));
    gtk_window_set_title (main_window, "Updater");
  

	g_signal_connect_swapped(G_OBJECT(main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	//g_timeout_add (100, update_ui, NULL);              
	g_object_unref(builder);

    gtk_widget_show(GTK_WIDGET(main_window));
    add_packages (list_box);


    gtk_main();

    return 0;
}

