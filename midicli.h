/*
 * midicast is released under the BSD 3-Clause license
 * read LICENSE for more info
 */

void init_midi_client(char *name, char *address, unsigned short port);
void midi_out(unsigned char data);
int poll_server(void);
