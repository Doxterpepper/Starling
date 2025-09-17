
#pragma once

namespace starling
{
    //
    // Plan is to give SoundPlayer a buffer like this. Another thread will continually fill this with data, SoundPlayer
    // will continually read from it to play sounds. This is our shared queue. Not sure if it's really necessary yet.
    //
    template <typename buffer_type>
    class PlaybackBuffer
    {
    public:
        /**
         * Push a data block to the queue. Take ownership of this buffer.
         */
        void push_buffer(std::vector< buffer_type >&& data)
        {
            buffer_queue.push_back(std::move(data));
        }

        void pop_front()
        {
            buffer_queue.pop_front();
        }

        const std::vector< buffer_type >& peek_front()
        {
            return buffer_queue.peek_front();
        }
    private:
        std::list< std::vector< buffer_type > > buffer_queue;
    };

}