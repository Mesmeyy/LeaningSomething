#pragma once
#include<iostream>
#include<string>
using namespace std;
class CStudentDTO:public CObject
{
private:
	int stuID;
	CString stuName;
	int stuScore;
public:
	CStudentDTO(int id, CString p, int score);
	CStudentDTO();
	CStudentDTO(int id);
	CStudentDTO(const CStudentDTO& );
	CStudentDTO& operator=(CStudentDTO&);

	void setStuID(int id);
	int getStuID() const ;
	
	void setStuName(CString p);
	CString getStuName() const;

	void setStuScore(int g);
	int getStuScore() const;
	
	virtual void Serialize(CArchive& ar) override;
	DECLARE_SERIAL(CStudentDTO)//第三等级宏
	//友元定义<< ;然后函数的实现写到对应.cpp或者本文件.h下面
};

