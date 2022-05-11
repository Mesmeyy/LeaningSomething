#include"stdafx.h"
#include"CLogicImpl.h"
#include<string>
using namespace std;
CLogicImpl::CLogicImpl()
{
	list = new CObArray;
}

CLogicImpl::~CLogicImpl()
{
}

CObArray *CLogicImpl::GetAllStudents()
{
    return list;
}

CStudentDTO CLogicImpl::FindByID(int ID)
{
	for (int i = 0; i < list->GetCount(); i++) {
		CStudentDTO* temp = (CStudentDTO*)list->GetAt(i);
		int id=temp->getStuID();
		if (id == ID) {
			return *temp;
		}
	}
    CStudentDTO *astu=new CStudentDTO(-1,_T(""),-1);
	return *astu;

}

bool CLogicImpl::AddStudentInfo(CStudentDTO & studentInfo)
{
	list->Add(new CStudentDTO(studentInfo.getStuID(), studentInfo.getStuName(), studentInfo.getStuScore()));
	return false;
}

void CLogicImpl::ReadStudentsFromFiletoList(CString name)
{
	CString fileName=name;

	CFile mFile;
	mFile.Open(fileName, CFile::modeRead);
	CArchive ar(&mFile, CArchive::load);


	list = (CObArray *)ar.ReadObject(RUNTIME_CLASS(CObArray));
	for (int i = 0; i < list->GetCount(); i++) {
		CStudentDTO* p = (CStudentDTO*)list->GetAt(i);
		int id = p->getStuID();
		CString name = p->getStuName();
		int score = p->getStuScore();
	}
	ar.Close();
}

void CLogicImpl::WriteStudentstoFromListtoFile(CString name)
{
	CString fileName=name;
	CFile mFile;
	mFile.Open(fileName, CFile::modeCreate | CFile::modeWrite);
	CArchive ar(&mFile, CArchive::store); //数据包做数据保存用
	ar.WriteObject(list);
	ar.Close();	
}


void CLogicImpl::DeleteStudentByID(int stuID)
{
	int index = -1;
	for (int i = 0; i < list->GetCount(); i++) {
		int id;
		CStudentDTO* temp = (CStudentDTO*)list->GetAt(i);
		id = temp->getStuID();
		if ( id == stuID) {
			index = i; break;
		}
	}

	list->RemoveAt(index);
}

void CLogicImpl::EditeStudentByID(CStudentDTO & stu)
{
	for (int i = 0; i < list->GetCount(); i++) {
		int id;
		CStudentDTO* temp = (CStudentDTO*)list->GetAt(i);
		id = temp->getStuID();
		if (id== stu.getStuID()) {
			list->SetAt(i,&stu);
		}
	}
	
}
int CLogicImpl::GetStudentCount()
{
	return list->GetCount();
}
