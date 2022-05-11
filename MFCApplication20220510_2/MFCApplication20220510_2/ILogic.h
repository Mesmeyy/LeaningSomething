#pragma once
#include "StudentDTO.h"
#include <string>
using namespace std;
class ILogic
{
public:
	virtual CObArray *GetAllStudents() = 0;
	virtual CStudentDTO FindByID(int ID)= 0;
	virtual void DeleteStudentByID(int stuID) = 0;
	virtual void EditeStudentByID(CStudentDTO& stu) = 0;
	virtual bool AddStudentInfo(CStudentDTO & studentInfo) = 0;
	virtual int  GetStudentCount() = 0;
	virtual void ReadStudentsFromFiletoList(CString) = 0;
	virtual void WriteStudentstoFromListtoFile(CString) = 0;
	virtual ~ILogic() = 0;
};

