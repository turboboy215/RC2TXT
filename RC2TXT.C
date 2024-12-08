/*Rolan's Curse 1/2/Ninja Taro (Sammy) (GB) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
long seqPtrs[4];
long songPtr;
int chanMask;
long bankAmt;
int foundTable = 0;
long firstPtr = 0;

unsigned static char* romData;

const char MagicBytes[5] = { 0x21, 0x31, 0xDD, 0x34, 0x21 };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Rolan's Curse 1/2/Ninja Taro (GB) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: RC2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);

			/*Try to search the bank for song table loader - Method 1: Mega Man 3/Bionic Commando*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes, 5)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i + 5;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
				}
			}

			if (foundTable == 1)
			{
				i = tableOffset - bankAmt;
				songNum = 1;
				while (ReadLE16(&romData[i]) > bankAmt && ReadLE16(&romData[i]) < (bankAmt * 2))
				{
					songPtr = ReadLE16(&romData[i]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					if (songPtr != 0)
					{
						song2txt(songNum, songPtr);
					}
					i += 2;
					songNum++;
				}
				printf("The operation was completed successfully!\n");
				return 0;
			}
			else
			{
				printf("ERROR: Magic bytes not found!\n");
				return -1;
			}


		}
	}
}

void song2txt(int songNum, long ptr)
{
	long channels[4] = { 0, 0, 0, 0 };
	long romPos = 0;
	long seqPos = 0;
	int curTrack = 0;
	int seqEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int chanSpeed = 0;
	int transpose = 0;
	int jumpPos = 0;
	int repeat = 0;
	int repeatPos = 0;
	int loopPos = 0;
	int headerEnd = 0;
	int frequency = 0;
	long macroPos = 0;
	long macroRet = 0;
	unsigned char command[3];
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;
	int isSFX = 0;
	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%i.txt!\n", songNum);
		exit(2);
	}
	else
	{
		romPos = ptr - bankAmt;
		if (romData[romPos] < 0x04)
		{
			isSFX = 1;
			fprintf(txt, "Type: SFX\n");
		}
		else
		{
			isSFX = 0;
			fprintf(txt, "Type: Music\n");
		}
		while (headerEnd == 0)
		{
			switch (romData[romPos])
			{
			case 0x00:
				curTrack = 0;
				break;
			case 0x01:
				curTrack = 1;
				break;
			case 0x02:
				curTrack = 2;
				break;
			case 0x03:
				curTrack = 3;
				break;
			case 0x04:
				curTrack = 0;
				break;
			case 0x05:
				curTrack = 1;
				break;
			case 0x06:
				curTrack = 2;
				break;
			case 0x07:
				curTrack = 3;
				break;
			case 0xFF:
				headerEnd = 1;
				break;
			default:
				break;
			}

			if (romData[romPos] != 0xFF)
			{
				channels[curTrack] = ReadLE16(&romData[romPos + 1]);
				fprintf(txt, "Channel %i: 0x%04X\n", (curTrack + 1), channels[curTrack]);
				romPos += 3;
			}

			fprintf(txt, "\n");
		}

		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			if (channels[curTrack] >= bankAmt)
			{
				fprintf(txt, "Channel %i:\n", (curTrack + 1));
				seqPos = channels[curTrack] - bankAmt;
				seqEnd = 0;

				while (seqEnd == 0)
				{
					command[0] = romData[seqPos];
					command[1] = romData[seqPos + 1];
					command[2] = romData[seqPos + 2];

					if (command[0] < 0xC0)
					{
						lowNibble = (command[0] >> 4);
						highNibble = (command[0] & 15);
						curNoteLen = lowNibble;
						curNote = highNibble;
						fprintf(txt, "Play note: %i, length: %i\n", curNote, curNoteLen);
						seqPos++;
					}

					else if (command[0] >= 0xC0 && command[0] < 0xD0)
					{
						fprintf(txt, "Select envelope: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] >= 0xD0 && command[0] < 0xE0)
					{
						fprintf(txt, "Set volume/envelope: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] == 0xE0)
					{
						fprintf(txt, "Set instrument: %01X, effect: %01X\n", command[1], command[2]);
						seqPos += 3;
					}

					else if (command[0] == 0xE1)
					{
						fprintf(txt, "Set sweep: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xE2)
					{
						frequency = command[1];
						fprintf(txt, "Set frequency: %01X\n", frequency);
						seqPos += 2;
					}

					else if (command[0] >= 0xE3 && command[0] <= 0xE8)
					{
						fprintf(txt, "Set envelope bit values: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] == 0xE9)
					{
						fprintf(txt, "Set noise tone: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xEA)
					{
						transpose = command[1];
						fprintf(txt, "Set transpose: %01X\n", transpose);
						seqPos += 2;
					}

					else if (command[0] == 0xEB)
					{
						fprintf(txt, "Transpose +1 octave\n");
						seqPos++;
					}

					else if (command[0] == 0xEC)
					{
						fprintf(txt, "Transpose -1 octave\n");
						seqPos++;
					}

					else if (command[0] >= 0xED && command[0] <= 0xEF)
					{
						fprintf(txt, "Reset channels?: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] >= 0xF0 && command[0] <= 0xF3)
					{
						fprintf(txt, "Set panning options?: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] == 0xF4)
					{
						fprintf(txt, "Set waveform: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xF5)
					{
						chanSpeed = 0x00;
						fprintf(txt, "Set channel speed to %01X\n", chanSpeed);
						seqPos++;
					}

					else if (command[0] == 0xF6)
					{
						chanSpeed = 0x0C;
						fprintf(txt, "Set channel speed to %01X\n", chanSpeed);
						seqPos++;
					}

					else if (command[0] == 0xF7)
					{
						chanSpeed = 0x18;
						fprintf(txt, "Set channel speed to %01X\n", chanSpeed);
						seqPos++;
					}

					else if (command[0] == 0xF8)
					{
						chanSpeed = 0x24;
						fprintf(txt, "Set channel speed to %01X\n", chanSpeed);
						seqPos++;
					}

					else if (command[0] == 0xF9)
					{
						loopPos = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Go to loop point: 0x%04X\n\n", loopPos);
						seqEnd = 1;
					}

					else if (command[0] == 0xFA)
					{
						jumpPos = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Jump to position: 0x%04X\n", jumpPos);
						seqPos += 3;
					}

					else if (command[0] == 0xFB)
					{
						repeat = command[1];
						fprintf(txt, "Set repeat for section: %01X\n", repeat);
						seqPos += 2;
					}

					else if (command[0] == 0xFC)
					{
						repeatPos = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Go to repeat point: 0x%04X\n", repeatPos);
						seqPos += 3;
					}

					else if (command[0] == 0xFD)
					{
						macroPos = ReadLE16(&romData[seqPos + 1]);
						macroRet = seqPos + 3;
						fprintf(txt, "Go to macro: 0x%04X\n", macroPos);
						seqPos += 3;
					}

					else if (command[0] == 0xFE)
					{
						fprintf(txt, "Exit macro\n");
						seqPos++;
					}

					else if (command[0] == 0xFF)
					{
						fprintf(txt, "End of track\n\n");
						seqEnd = 1;
					}

				}
			}
		}
		fclose(txt);
	}
}