// GomunseoDBDlg.h: 헤더 파일
//

#pragma once

#include <vector>
#include <set> // 통계 계산용 (중복 제거)
using namespace std;

// [평가기준] SCharInfo 구조체 (5점)
// 기존 CharData를 이걸로 대체합니다.
// GomunseoDBDlg.h 안의 구조체 부분을 이걸로 교체하세요.

// [수정된 구조체] 생성자 추가됨
struct SCharInfo {
	CString m_char;
	int m_type;
	int m_sheet;
	int m_sx, m_sy;
	int m_line, m_order;
	int m_width, m_height;

	// ★ 이 부분이 추가되어야 "초기화되지 않았습니다" 에러가 사라집니다.
	SCharInfo() {
		m_type = 0; m_sheet = 0;
		m_sx = 0; m_sy = 0;
		m_line = 0; m_order = 0;
		m_width = 0; m_height = 0;
	}
};

// [평가기준] CTypeDB 클래스 (10점)
// 데이터를 관리하는 전용 클래스입니다.
class CTypeDB {
public:
	int m_nSheet; // 전체 장 수
	int m_nChar;  // 전체 글자 수
	vector<SCharInfo> m_Chars; // 데이터 배열 (기존 m_dbList 역할)

	CTypeDB() {
		m_nSheet = 0;
		m_nChar = 0;
	}

	// [평가기준] CSV 파일 읽기 함수 (TRUE/FALSE 리턴)
	BOOL ReadCSVFile(CString filename) {
		CStdioFile file;
		// 파일 열기 시도
		if (!file.Open(filename, CFile::modeRead | CFile::typeText))
			return FALSE;

		CString strLine;
		file.ReadString(strLine); // 첫 줄(헤더) 건너뛰기

		m_Chars.clear();
		int maxSheet = 0;

		// 파일 끝까지 읽기
		while (file.ReadString(strLine)) {
			SCharInfo info;
			CString temp;

			// 콤마(,) 기준으로 데이터 파싱
			AfxExtractSubString(info.m_char, strLine, 0, ',');
			AfxExtractSubString(temp, strLine, 1, ','); info.m_type = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 2, ','); info.m_sheet = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 3, ','); info.m_sx = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 4, ','); info.m_sy = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 5, ','); info.m_line = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 6, ','); info.m_order = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 7, ','); info.m_width = _ttoi(temp);
			AfxExtractSubString(temp, strLine, 8, ','); info.m_height = _ttoi(temp);

			// 최대 장 번호 갱신
			if (info.m_sheet > maxSheet) maxSheet = info.m_sheet;

			// 리스트에 추가
			m_Chars.push_back(info);
		}
		file.Close();

		// 전체 통계 저장
		m_nSheet = maxSheet;
		m_nChar = (int)m_Chars.size();
		return TRUE;
	}
};

// CGomunseoDBDlg 대화 상자
class CGomunseoDBDlg : public CDialogEx
{
	// 생성입니다.
public:
	CGomunseoDBDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOMUNSEODB_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


	// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	// ==========================================
	// [수정] 교수님 요구사항에 맞춘 변수들
	// ==========================================

	CTypeDB m_TypeDB;      // ★ 데이터 관리 클래스 (기존 m_dbList 대체)
	CString m_strRootPath; // ★ 선택한 책 폴더 경로 저장 (예: C:\...\GomunseoDB)

	int m_nCurrentSheet;   // 현재 페이지
	int m_nSelectedIdx;    // 선택된 글자 인덱스

	// 스핀 컨트롤용 (같은 글자 모음)
	vector<int> m_sameCodeList;
	int m_nTypeViewIdx;

	// ==========================================
	// 함수 선언
	// ==========================================
	void UpdateInfoLabel();      // 통계 갱신 함수
	void UpdateComponentList();  // [평가 9] 구성 글자 리스트 갱신 함수

	// 이벤트 핸들러 함수들
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult); // 스핀 컨트롤
	// ★ [추가] 장 번호 스핀 컨트롤 함수
	afx_msg void OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult);

	// [새로 추가됨]
	afx_msg void OnBnClickedBtnOpenFolder();
	afx_msg void OnNMClickListComponents(NMHDR* pNMHDR, LRESULT* pResult); // 리스트 클릭
	afx_msg void OnBnClickedButton1();
};