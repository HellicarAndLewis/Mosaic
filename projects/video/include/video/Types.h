/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/
#ifndef VIDEO_TYPES_H
#define VIDEO_TYPES_H

#define VID_EVENT_START_PLAYBACK 0x01                         /* this jitter buffer has enough frames to start playback; playback will start now */
#define VID_EVENT_STOP_PLAYBACK 0x02                          /* we've played all the frames from the jitter buffer. */
#define VID_EVENT_EOF 0x03                                    /* the eof has been read; doesn't mean we've played back everything */
#define VID_EVENT_TIMEOUT 0x04                                /* when we can't connect to the remote stream and we timeout in the interrupt callback (see Stream.cpp) this event is fired. */

typedef void(*video_on_event)(int event, void* user);         /* gets called when the playback e.g. starts, stops or eof happens. */

#endif
