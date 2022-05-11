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
	DECLARE_SERIAL(CStudentDTO)//�����ȼ���
	//��Ԫ����<< ;Ȼ������ʵ��д����Ӧ.cpp���߱��ļ�.h����
};

