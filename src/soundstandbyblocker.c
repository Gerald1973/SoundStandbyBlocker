#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 1024
#define PI 3.14159265
#define SAMPLE_RATE 44100
#define SLEEP_DURATION 60

static const pa_sample_spec ss = {
    .format = PA_SAMPLE_S16LE,
    .rate = SAMPLE_RATE,
    .channels = 2};

/**
 * @brief Structure representing a sound frame.
 *
 * This structure is used to store a sound frame, with information about its size, the frame data itself, and an index for the next element.
 *
 * @param size The size of the sound frame. This is the number of samples in the frame.
 * @param trame A pointer to an array containing the sound frame data. Each element in the array is a sound sample.
 * @param next An index for the next element to process in the sound frame. If next equals 1, it means there are more frames to process.
 */
struct SoundTrame
{
    int size;
    short *trame;
    int next;
};

/**
 * @brief Generates a sine wave.
 *
 * This function generates a sine wave of a specified frequency and volume.
 * The wave is generated for a duration of one second, with a sample rate defined by SAMPLE_RATE.
 * The wave is generated for a number of channels defined by ss.channels.
 *
 * @param frequency The frequency of the sine wave in Hz.
 * @param maxVolume The maximum volume of the sine wave.
 * @return A pointer to a static array containing the samples of the sine wave.
 */
short *sineWave(int frequency, int maxVolume)
{
    static short results[SAMPLE_RATE * 2];
    int i;
    double t;
    double sample;
    int amplitude = 0;

    for (i = 0; i < SAMPLE_RATE * ss.channels; i = i + ss.channels)
    {
        t = (double)i / (double)SAMPLE_RATE;
        sample = sin(frequency * 2 * PI * t);
        amplitude = (short)(sample * maxVolume);
        for (int j = 0; j < ss.channels; j++)
        {
            results[i + j] = amplitude;
        }
    }
    return results;
}

/**
 * @brief Creates a SoundTrame structure from a sample array.
 *
 * This function creates a SoundTrame structure from a given sample array. The SoundTrame structure contains a portion of the sample array starting from a specified index. The size of the portion is determined by the BUFSIZE constant. If the remaining samples in the array are less than BUFSIZE, all remaining samples are included in the SoundTrame structure.
 *
 * @param start The starting index in the sample array for the SoundTrame.
 * @param sample A pointer to the sample array.
 * @param sampleSize The total number of samples in the sample array.
 * @return A SoundTrame structure containing a portion of the sample array.
 */
struct SoundTrame trame(int start, short *sample, int sampleSize)
{
    struct SoundTrame result;
    int i;
    if (start + BUFSIZE < sampleSize)
    {
        result.size = BUFSIZE;
        result.trame = sample + start;
        result.next = 1;
    }
    else
    {
        int reminder = (start + BUFSIZE) - sampleSize;
        result.size = reminder;
        result.trame = sample + start;
        result.next = 0;
    }
    return result;
}

/**
 * @brief Plays a sound sample using the PulseAudio library.
 *
 * This function plays a sound sample using the PulseAudio library. The sound sample is divided into frames using the `trame` function, and each frame is played back using the `pa_simple_write` function from the PulseAudio library. If an error occurs during playback, the function stops playing the sound and returns an error.
 *
 * @param sample A pointer to the sound sample to be played.
 * @return An integer indicating whether the function succeeded or failed. Returns 0 if the function succeeded, and 1 if it failed.
 */
int playsound(short *sample)
{
    struct SoundTrame tmp;
    pa_simple *s = NULL;
    int error;
    int failed = 0;
    if ((s = pa_simple_new(NULL, "soundstandbyblocker", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error)))
    {
        int c = 0;
        do
        {
            tmp = trame(BUFSIZE * c, sample, SAMPLE_RATE);
            if (pa_simple_write(s, tmp.trame, tmp.size * sizeof(short), &error) < 0)
            {
                fprintf(stderr, __FILE__ ": pa_simple_write() failed: %s\n", pa_strerror(error));
                if (s)
                {
                    pa_simple_free(s);
                }
                failed = 1;
            }
            c++;
        } while (failed == 0 && tmp.next == 1);
        if (failed == 0)
        {
            if (pa_simple_drain(s, &error) < 0)
            {
                failed = 1;
            }
        }
        pa_simple_free(s);
    }
    else
    {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n", pa_strerror(error));
        failed = 1;
    }
    return failed;
}

int main(int argc, char *argv[])
{
    printf("Send a tick every n seconds to wake up your speaker.\n");
    short *sample;
    int ret = 0;
    sample = sineWave(50, 250);
    while (1 == 1)
    {
        ret = playsound(sample);
        sleep(SLEEP_DURATION);
    }
    return ret;
}
