#if !defined(AFX_CLIENT_H__2D721F56_3253_48C2_8EED_FE0181AD999A__INCLUDED_)
#define AFX_CLIENT_H__2D721F56_3253_48C2_8EED_FE0181AD999A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Cclient
{
public:
	virtual int process_events(int) = 0;
	virtual ~Cclient();
};

#endif // !defined(AFX_CLIENT_H__2D721F56_3253_48C2_8EED_FE0181AD999A__INCLUDED_)
