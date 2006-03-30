#if !defined(AFX_DATA_COUNTER_H__F3C125B1_D612_41A0_B2D2_AB9240FDC72E__INCLUDED_)
#define AFX_DATA_COUNTER_H__F3C125B1_D612_41A0_B2D2_AB9240FDC72E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Cdata_counter
{
public:
	int rate(time_t) const;
	void add(int, time_t);
	Cdata_counter();
private:
	void update_rate(time_t);

	int m_got;
	int m_rate;
	time_t m_start_time;
};

#endif // !defined(AFX_DATA_COUNTER_H__F3C125B1_D612_41A0_B2D2_AB9240FDC72E__INCLUDED_)
