#pragma once
#include "StudentDTO.h"
#include "ILogic.h"
#include"resource.h"
class CLogicImpl : public ILogic
{
private:
	CObArray *list;
public:
	CLogicImpl();
	~CLogicImpl();

	CObArray * GetAllStudents() override;
    CStudentDTO FindByID(int ID) override;
	int  GetStudentCount() override;
	void DeleteStudentByID(int stuID) override;
	void EditeStudentByID(CStudentDTO& stu) override;
	bool AddStudentInfo(CStudentDTO & studentInfo) override;
	void ReadStudentsFromFiletoList(CString) override;
	void WriteStudentstoFromListtoFile(CString) override;
};

