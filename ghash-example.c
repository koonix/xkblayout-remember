/* compile: cc ghash-example.c $(pkg-config --cflags --libs glib-2.0) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

/** This prints out a key/value combination, for use in "g_hash_foreach".
  **/
void printKeyValue( gpointer key, gpointer value, gpointer userData ) {
   int realKey = GPOINTER_TO_INT( key );
   char* realValue = (char*)value;

   printf( "%d => %s\n", realKey, realValue );
   return;
}

/** The main function.
  **/
int main( int argc, char** argv ) {
   //! This is our map.
   GHashTable* map = NULL;
   //! This is the pointer to our value data.
   char* value = NULL;

   // Create the GHashTable.
   map = g_hash_table_new_full(
      g_direct_hash, g_direct_equal, //< This is an integer hash.
      NULL, //< There is nothing to free for our key.
      free //< Call "free" on the value (made with "strdup").
   );

   // Insert some values.
   // 7 => seven
   value = strdup( "seven" );
   g_hash_table_insert( map, GINT_TO_POINTER( 7 ), value );

   // 10 => ten
   value = strdup( "ten" );
   g_hash_table_insert( map, GINT_TO_POINTER( 10 ), value );

   // 9001 => over nine-thousand!
   value = strdup( "over nine-thousand!" );
   g_hash_table_insert( map, GINT_TO_POINTER( 9001 ), value );

   // Print out the contents of the map.
   g_hash_table_foreach( map, printKeyValue, NULL );

   // Destroy the map.
   g_hash_table_destroy( map );
   map = NULL;

   return 0;
}
