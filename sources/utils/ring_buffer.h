#pragma once
#include <memory>
#include <string.h>


template<typename T>
class RingBuffer
{
//    Ring buffer for numpy array. Has fixed max size, if new data is pushed, old gets deleted.
//    No resizing/copying envolved
	enum
	{
		min_size = 0x100
	};

	std::shared_ptr<T> m_data;
	int m_reserved;
	int m_frame_size;
	int m_frame_offset;
	int m_history_size;

	bool Resize(size_t new_size, int move_offset)
	{
		if (new_size > m_reserved || move_offset != 0)
		{
			uint64_t v = new_size + 1;

			v--;
			v |= v >> 1U;
			v |= v >> 2U;
			v |= v >> 4U;
			v |= v >> 8U;
			v |= v >> 16U;
			v |= v >> 32U;
			v++;
			auto old_reserved = m_reserved;
			m_reserved = static_cast<size_t>(v);
			auto old_data = m_data;
			m_data.reset(new T[m_reserved]);
			if (move_offset >= 0)
				memcpy(m_data.get() + sizeof(T) * move_offset, old_data.get(), sizeof(T) * old_reserved);
			else
				memcpy(m_data.get(), old_data.get() + sizeof(T) * move_offset, sizeof(T) * (old_reserved - move_offset));
		}
		return m_data != nullptr;
	}

public:


    RingBuffer()
    {
		m_data.reset(new T[min_size]);
		m_reserved = min_size;

//	    //
//	    // Args:
//	    //     maxlen (int): max size of ring buffer. When buffer grows beyond this size, old data gets discarded.
//	    //     shape (List[int]): shape of entry in the buffer.
//	    self.size = maxlen
//	    self.padding = self.size
//	    self.buffer = np.zeros([self.size + self.padding] + shape)
//	    self.counter = 0
//	    self.total_counter = 0
    }

    void SetFrame(int history_size, int frame_size)
    {
		int reserve = (history_size + frame_size) * 5;

		int history_beginning = m_frame_offset - history_size;
		int frame_end = m_frame_offset + frame_size;
		int move_offset = history_beginning < 0 ? -history_beginning : 0;
		move_offset = frame_end > reserve ? -history_beginning : 0;

		Resize(reserve, move_offset);
		m_frame_offset += move_offset;
    }

    void AdvanceFrame(int64_t advance)
	{

	}

//        # type: (np.ndarray) -> None
//        """
//        Args:
//            data (np.ndarray): new data to append to the buffer. If dim 0 of the buffer vecomes greater than
//            maxlen, ld data gets discarded.
//        """
//        data = data[-self.padding:]
//        n = len(data)
//        if self.padding-self.counter < n:
//            self.buffer[:self.size] = self.buffer[self.counter:][:self.size]
//            self.counter = 0
//        self.buffer[self.counter + self.size:][:n] = data
//        self.counter += n
//        self.total_counter += n
//
//    def view(self):
//        # type: () -> np.ndarray
//        """
//        Returns:
//            np.ndarray of the view onto the ring buffer as a flat array.
//        """
//        size = min(self.total_counter, self.size)
//        return self.buffer[self.counter + self.size - size:][:size]
}