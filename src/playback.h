#ifndef __PLAYBACK_H__
#define __PLAYBACK_H__


/////// Global Constants //////////////////////////////////////////////////////////////////////////

#define PLAYBACK_FREQUENCY  15360 //playback speed in hz (eg. 15360, 30720)


/////// Function Prototypes ///////////////////////////////////////////////////////////////////////

void playback_init();
void playback_start();
void playback_stop();


#endif // __PLAYBACK_H__
