/*
 * Copyright (c) 2013-2014, Ixonos Denmark ApS
 * Copyright (c) 2012-2014, Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <string.h>
#include "sensact/plugin-manager.h"
#include "sensact/plugin.h"
#include "sensact/list.h"
#include "sensact/debug.h"

list_p plugin_list;

struct plugin_item_t
{
    char name[256];
    void *handle;
};

struct plugin_item_t plugin_item;
struct plugin_item_t *plugin_item_p;

static void plugin_print_info(struct sa_plugin_t *plugin, char *plugin_name)
{
    int i;

    // Print plugin information
    debug_printf("\n");
    debug_printf("Plugin information\n");
    debug_printf("------------------\n");
    debug_printf(" Name: %s\n", plugin_name);
    debug_printf(" Version: %s\n", plugin->version);
    debug_printf(" Description: %s\n", plugin->description);
    debug_printf(" Author: %s\n", plugin->author);
    debug_printf(" License: %s\n", plugin->license);
    debug_printf("\n");
}

int plugin_load(char *name)
{
    char filename[256];
    struct sa_plugin_t * (*plugin_register)(void);
    int (*plugin_load)();
    struct sa_plugin_t *plugin;
    char *error;

    debug_printf("Loading plugin %s\n", name);

    // Fill in name in list element
    strcpy(plugin_item.name, name);

    // Check that plugin is not already loaded
    list_iter_p iter = list_iterator(plugin_list, FRONT);
    while (list_next(iter) != NULL)
    {
        plugin_item_p = (struct plugin_item_t *)list_current(iter);
        if (strcmp(plugin_item_p->name, name) == 0)
        {
            printf("Error: Plugin already loaded!\n");
            return -1;
        }
    }

    // Add plugin location 
    sprintf(filename, "sensact-%s.so", name);

    // Open plugin
    plugin_item.handle = dlopen(filename, RTLD_LAZY);
    if (!plugin_item.handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        return -1;
    }
    else
    {
        // Add plugin to list of loaded plugins
        list_add(plugin_list, &plugin_item, sizeof(plugin_item));

        // Call plugin_register()
        plugin_register = dlsym(plugin_item.handle, "plugin_register");
        if ((error = dlerror()) != NULL)
            fprintf(stderr, "%s\n", error);
        plugin = (*plugin_register)();

        // Print plugin information
        plugin_print_info(plugin, name);

        // Call plugin load callback (if defined)
        if (plugin->load != NULL)
        {
            plugin_load = plugin->load;
            (*plugin_load)();
        }
    }

    return 0;
}

int plugin_unload(char *name)
{
    struct sa_plugin_t * (*plugin_register)(void);
    int (*plugin_unload)(void);
    struct sa_plugin_t *plugin;
    char *error;
    bool found = false;

    debug_printf("Unloading plugin %s\n", name);

    // Check that the plugin is loaded
    list_iter_p iter = list_iterator(plugin_list, FRONT);
    while (list_next(iter) != NULL)
    {
        plugin_item_p = (struct plugin_item_t *)list_current(iter);
        if (strcmp(plugin_item_p->name, name) == 0)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        printf("Error: Plugin not found!\n");
        return -1;
    }

    // Call plugin_register()
    plugin_register = dlsym(plugin_item_p->handle, "plugin_register");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }
    plugin = (*plugin_register)();

    // Call plugin unload callback (if defined)
    if (plugin->unload != NULL)
    {
        plugin_unload = plugin->unload;
        (*plugin_unload)();
    }

    // Unload plugin
    if (dlclose(plugin_item_p->handle))
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    // Remove plugin from list of loaded plugins
    list_pluck(plugin_list, iter->current);

    return 0;
}

void plugin_manager_start(void)
{
    // Initialize plugin list
    plugin_list = create_list();
}
