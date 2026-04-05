# EEP522-Project
Repo the project of my class

# Hands-Free MP3

This code is for my class project of a hands-free MP3 player. For this project I wanted to create something that uses wireless communication, so this player uses Bluetooth (BT) to connect to a speaker. I won't go into large detail about the whole project, but will explain the basics for anyone who finds this repo for reference.

First the hardware that is needed. The code is made for a Raspberry Pi, but it should run on any linux system. Next is any BT speaker. The speaker I own has BT-LE, which I recommend it is kinda nice to not have to actually ‘power on’ the speaker. Last, and most important, a gesture senser. I used the Grove – Gesture from Seeed Studio, but I think this code will work with any PAJ7620U2 sensor. So as long as that sensor is used, this code could be a drop-and-place for you.

Now I did NOT build a media player, I used the mpg123 player. There are other players you can use, it just seemed to be the one people on forums recommended to use. I can say that the advantage I had with it was that it has a ‘remote’ mode. We need to have the music playing in a background task and remote can help with that. Normally you would use ‘&’ or ‘nohup’ for background tasks, but they won’t work the way we want them to. I won’t go deep into them, but I do not recommend them.

Now your question “why do we want to run music in background?” is a good question to have. My view for my project is that if music is player, I will need to interrupt it to control it. But we don’t want to pause the music to control it (such as change the volume). Now maybe I was wrong, maybe the controls happen so fast that the human ear won’t notice the slight pause when doing a ‘control’ task. IDK, I didn’t try it, I wanted to avoid that situation altogether. I was running low on time, but this might have been a perfect situation to learn and apply ‘threads’.

The only main control I wanted to have was next song, restart song, pause/play, and volume control. The most important one surprisingly is volume control. The actual song is still controlled in mpg123, BUT NOT volume. Due to reasons I didn’t look heavily into, the only way I found to change the volume over BT is with WirePlumber. There are several different controls for signal processing (the ALSA and PCM) and it changes between wired vs wireless. Since BT is clearly wireless, the program Amixer will not work for volume. WirePlumber HAS to be used for wireless volume.

Side note: to connect to BT speaker use bluetoothctl. The instructions are straightforward, but you might run into the “Failed to set power on: org.bluez.Error.Failed” error. If you did, run this: “rfkill unblock Bluetooth”. It is not mentioned on all sites, but you do need to run it. 

This should cover the main points for my method and why I did certain methods over others. Now maybe YOU have better ideas on different approaches or why methods weren’t working – I’d love to hear them. It would be cool to learn some proper explanations. But I wrote this to help a person who might be in the same position as me – wanting to have wireless music, but every example online does something different that won’t work the situation that I’m trying to do! 

