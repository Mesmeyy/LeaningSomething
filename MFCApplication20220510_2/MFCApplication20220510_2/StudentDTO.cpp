#include"stdafx.h"
#include"StudentDTO.h"

IMPLEMENT_SERIAL(CStudentDTO,CObject, 2)
CStudentDTO::CStudentDTO(int id, CString p, int score)
{
	stuID = id;
	stuName = p;
	stuScore = score;
}
CStudentDTO::CStudentDTO()
{

}
CStudentDTO::CStudentDTO(int id)
{
	this->stuID = id;

}
CStudentDTO::CStudentDTO(const CStudentDTO & p)
{
	stuID = p.getStuID();
	stuName = p.getStuName();
	stuScore = p.getStuScore();
}
CStudentDTO & CStudentDTO::operator=(CStudentDTO &a)
{
	stuID = a.getStuID();
	stuName = a.getStuName();
	stuScore = a.getStuScore();
	return *this;
	// TODO: 在此处插入 return 语句
}
void CStudentDTO::setStuID(int i) 
{
	stuID = i;
}
int CStudentDTO::getStuID() const
{
	return stuID;
}
void CStudentDTO::setStuName(CString p)
{
	stuName = p;
}
void CStudentDTO::setStuScore(int g)
{
	stuScore = g;
}
int CStudentDTO::getStuScore() const
{
	return stuScore;
}
CString CStudentDTO::getStuName() const
{
	return stuName;
}
//bool operator==(const CStudentDTO& a, const CStudentDTO& b)
//{
//	if (a.stuID == b.stuID)
//		return true;
//	else
//		return false;
//}

void CStudentDTO::Serialize(CArchive& ar)//数据包，是系统传入的
{
	if (ar.IsStoring())
	{
		ar <<stuID;
		ar <<stuName;
		ar <<stuScore;
	}
	else
	{
		ar >> stuID;
		ar >> stuName;
		ar >> stuScore;
	}
}