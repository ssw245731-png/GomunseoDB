// GomunseoDBDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "GomunseoDB.h"
#include "GomunseoDBDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// =================================================================
// CGomunseoDBDlg 대화 상자 (여기서부터 메인 코드입니다)
// =================================================================

CGomunseoDBDlg::CGomunseoDBDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GOMUNSEODB_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// [초기화] 변수 초기값 설정 (평가 1번: 책 선택 이전 초기화)
	m_nCurrentSheet = 1;
	m_nSelectedIdx = -1;
	m_nTypeViewIdx = 0;
	m_strRootPath = _T(""); // 아직 책 폴더 선택 안 함
}

void CGomunseoDBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGomunseoDBDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// 마우스 클릭
	ON_WM_LBUTTONDOWN()

	// ★ [추가] 돋보기(찾기) 버튼 연결
	ON_BN_CLICKED(IDC_BTN_OPEN_FOLDER, &CGomunseoDBDlg::OnBnClickedBtnOpenFolder)
	// [평가 7] 스핀 컨트롤
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TYPE, &CGomunseoDBDlg::OnDeltaposSpinType)
	// [평가 10] 리스트 컨트롤 클릭
	ON_NOTIFY(NM_CLICK, IDC_LIST_COMPONENTS, &CGomunseoDBDlg::OnNMClickListComponents)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHEET, &CGomunseoDBDlg::OnDeltaposSpinSheet)
END_MESSAGE_MAP()


// CGomunseoDBDlg 메시지 처리기

BOOL CGomunseoDBDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴 설정 (기본 코드)
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘

	// ==========================================
	// [추가] 리스트 컨트롤 초기화 (평가 9번 준비)
	// ==========================================
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_COMPONENTS);
	if (pList) {
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		pList->InsertColumn(0, _T("장"), LVCFMT_LEFT, 50);
		pList->InsertColumn(1, _T("행"), LVCFMT_LEFT, 50);
		pList->InsertColumn(2, _T("번"), LVCFMT_LEFT, 50);
		pList->InsertColumn(3, _T("유니코드"), LVCFMT_LEFT, 100);
	}

	return TRUE;
}

void CGomunseoDBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// =================================================================
// [평가 2, 3, 11] 책 불러오기 버튼 (폴더 선택 + 모래시계)
// =================================================================


void CGomunseoDBDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();

		if (m_strRootPath.IsEmpty()) return;

		// 1. 이미지 로드
		CImage img;
		CString strPath;
		bool bLoaded = false;

		strPath.Format(_T("%s\\01_scan\\%03d.jpg"), (LPCTSTR)m_strRootPath, m_nCurrentSheet);
		if (img.Load(strPath) == S_OK) bLoaded = true;
		else {
			strPath.Format(_T("%s\\01_scan\\%d.jpg"), (LPCTSTR)m_strRootPath, m_nCurrentSheet);
			if (img.Load(strPath) == S_OK) bLoaded = true;
		}

		if (!bLoaded) return;

		// 2. [핵심 변경] 액자 컨트롤의 "위치"만 가져옵니다.
		CWnd* pWndMain = GetDlgItem(IDC_PIC_MAIN_VIEW);
		if (!pWndMain) return;

		CRect rectMain;
		pWndMain->GetWindowRect(&rectMain); // 모니터 기준 절대 좌표
		ScreenToClient(&rectMain);          // 다이얼로그 기준 상대 좌표로 변환

		// 3. [핵심 변경] 액자(pWndMain)가 아니라 '다이얼로그(this)'에 그립니다.
		CClientDC dc(this);

		// 이미지 그리기
		dc.SetStretchBltMode(COLORONCOLOR);
		img.Draw(dc.m_hDC, rectMain);

		// 4. 비율 계산 (이미지 크기 vs 액자 영역 크기)
		double ratioX = (double)rectMain.Width() / (double)img.GetWidth();
		double ratioY = (double)rectMain.Height() / (double)img.GetHeight();

		// 5. 박스 그리기 (이제 dc는 다이얼로그의 것입니다)
		CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
		CPen greenPen(PS_SOLID, 1, RGB(0, 255, 0));
		CPen* pOldPen = dc.SelectObject(&greenPen);

		// 녹색 박스 그리기
		// 주의: 박스 좌표도 '액자 시작점(rectMain.left, top)'만큼 더해줘야 합니다!
		for (const auto& data : m_TypeDB.m_Chars) {
			if (data.m_sheet == m_nCurrentSheet) {
				int left = rectMain.left + (int)(data.m_sx * ratioX);
				int top = rectMain.top + (int)(data.m_sy * ratioY);
				int right = rectMain.left + (int)((data.m_sx + data.m_width) * ratioX);
				int bottom = rectMain.top + (int)((data.m_sy + data.m_height) * ratioY);
				dc.Rectangle(left, top, right, bottom);
			}
		}

		// 빨간 박스 그리기
		if (m_nSelectedIdx != -1) {
			CPen redPen(PS_SOLID, 3, RGB(255, 0, 0));
			dc.SelectObject(&redPen);
			SCharInfo& data = m_TypeDB.m_Chars[m_nSelectedIdx];
			if (data.m_sheet == m_nCurrentSheet) {
				int left = rectMain.left + (int)(data.m_sx * ratioX);
				int top = rectMain.top + (int)(data.m_sy * ratioY);
				int right = rectMain.left + (int)((data.m_sx + data.m_width) * ratioX);
				int bottom = rectMain.top + (int)((data.m_sy + data.m_height) * ratioY);
				dc.Rectangle(left, top, right, bottom);
			}
		}

		dc.SelectObject(pOldPen);
		dc.SelectObject(pOldBrush);
	}
}

HCURSOR CGomunseoDBDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// =================================================================
// 마우스 클릭 이벤트 (글자 선택)
// =================================================================
void CGomunseoDBDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_strRootPath.IsEmpty()) {
		CDialogEx::OnLButtonDown(nFlags, point);
		return;
	}

	// 1. 클릭한 곳이 '액자' 안인지 확인
	CWnd* pWndMain = GetDlgItem(IDC_PIC_MAIN_VIEW);
	if (!pWndMain) return;

	CRect rectMainScreen;
	pWndMain->GetWindowRect(&rectMainScreen); // 절대 좌표
	ScreenToClient(&rectMainScreen); // 상대 좌표로 변환

	if (!rectMainScreen.PtInRect(point)) {
		CDialogEx::OnLButtonDown(nFlags, point);
		return;
	}

	// 2. 좌표 변환 (액자 기준)
	CPoint ptInPic = point;
	ptInPic.x -= rectMainScreen.left;
	ptInPic.y -= rectMainScreen.top;

	// 3. 이미지 정보 가져오기 (비율 계산용)
	CImage img;
	CString strPath;
	strPath.Format(_T("%s\\01_scan\\%03d.jpg"), (LPCTSTR)m_strRootPath, m_nCurrentSheet);
	if (img.Load(strPath) != S_OK) {
		strPath.Format(_T("%s\\01_scan\\%d.jpg"), (LPCTSTR)m_strRootPath, m_nCurrentSheet);
		if (img.Load(strPath) != S_OK) return;
	}

	CRect rectMainClient;
	pWndMain->GetClientRect(&rectMainClient);

	double ratioX = (double)img.GetWidth() / (double)rectMainClient.Width();
	double ratioY = (double)img.GetHeight() / (double)rectMainClient.Height();

	// 4. 원본 좌표로 역계산
	int originX = (int)(ptInPic.x * ratioX);
	int originY = (int)(ptInPic.y * ratioY);

	// 5. 검색 시작
	int oldSelected = m_nSelectedIdx;
	m_nSelectedIdx = -1;

	for (int i = 0; i < m_TypeDB.m_Chars.size(); i++) {
		if (m_TypeDB.m_Chars[i].m_sheet != m_nCurrentSheet) continue;

		SCharInfo& data = m_TypeDB.m_Chars[i];

		if (originX >= data.m_sx && originX <= (data.m_sx + data.m_width) &&
			originY >= data.m_sy && originY <= (data.m_sy + data.m_height))
		{
			m_nSelectedIdx = i; // 찾음!
			Invalidate(); UpdateWindow(); // 화면 갱신

			// ==========================================
			// [복구] 여기서부터 기존 로직을 다시 넣었습니다.
			// ==========================================

			// 1. 텍스트 정보 갱신
			CString strCode, strPos;
			strCode.Format(_T("%s"), (LPCTSTR)data.m_char);
			strPos.Format(_T("%d장 %d행 %d번"), data.m_sheet, data.m_line, data.m_order);
			SetDlgItemText(IDC_STATIC_CODE, strCode);
			SetDlgItemText(IDC_STATIC_POS, strPos);

			// 2. 이미지(03_type) 갱신
			CImage typeImg;
			CString strFolderPath, strSearchPath;
			strFolderPath.Format(_T("%s\\03_type\\%s\\%d"), (LPCTSTR)m_strRootPath, (LPCTSTR)data.m_char, data.m_type);
			strSearchPath.Format(_T("%s\\*.png"), (LPCTSTR)strFolderPath);

			CFileFind finder;
			if (finder.FindFile(strSearchPath)) {
				finder.FindNextFile();
				if (typeImg.Load(finder.GetFilePath()) == S_OK) {
					CWnd* pWnd = GetDlgItem(IDC_PIC_TYPE);
					if (pWnd) {
						CDC* pDC = pWnd->GetDC();
						CRect rect; pWnd->GetClientRect(&rect);
						pDC->FillSolidRect(rect, RGB(255, 255, 255));
						typeImg.Draw(pDC->m_hDC, rect);
						pWnd->ReleaseDC(pDC);
					}
				}
			}

			// 3. 스핀 컨트롤 리스트 갱신
			m_sameCodeList.clear();
			m_nTypeViewIdx = 0;
			for (int k = 0; k < m_TypeDB.m_Chars.size(); k++) {
				if (m_TypeDB.m_Chars[k].m_char == data.m_char) {
					m_sameCodeList.push_back(k);
					if (k == i) m_nTypeViewIdx = (int)m_sameCodeList.size() - 1;
				}
			}
			CString strTypeInfo;
			strTypeInfo.Format(_T("%d / %d"), m_nTypeViewIdx + 1, (int)m_sameCodeList.size());
			SetDlgItemText(IDC_STATIC_TYPE_INFO, strTypeInfo);

			// 4. 구성 글자 리스트 갱신
			UpdateComponentList();

			break;
		}
	}

	if (m_nSelectedIdx == -1 && oldSelected != -1) {
		Invalidate();
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

// =================================================================
// [평가 7] 스핀 컨트롤 (같은 활자 돌려보기)
// =================================================================
void CGomunseoDBDlg::OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = (LPNMUPDOWN)pNMHDR;

	if (m_sameCodeList.empty()) {
		*pResult = 0; return;
	}

	// 인덱스 이동
	if (pNMUpDown->iDelta < 0) m_nTypeViewIdx++;
	else m_nTypeViewIdx--;

	// 순환
	if (m_nTypeViewIdx < 0) m_nTypeViewIdx = (int)m_sameCodeList.size() - 1;
	if (m_nTypeViewIdx >= m_sameCodeList.size()) m_nTypeViewIdx = 0;

	// 데이터 갱신
	m_nSelectedIdx = m_sameCodeList[m_nTypeViewIdx];
	SCharInfo& data = m_TypeDB.m_Chars[m_nSelectedIdx];

	// 페이지 이동
	if (m_nCurrentSheet != data.m_sheet) {
		m_nCurrentSheet = data.m_sheet;
		UpdateInfoLabel(); // 통계 갱신
	}

	Invalidate();
	UpdateWindow();

	// 정보창 갱신
	CString strCode, strPos, strTypeInfo;
	strCode.Format(_T("%s"), (LPCTSTR)data.m_char);
	strPos.Format(_T("%d장 %d행 %d번"), data.m_sheet, data.m_line, data.m_order);
	strTypeInfo.Format(_T("%d / %d"), m_nTypeViewIdx + 1, (int)m_sameCodeList.size());

	SetDlgItemText(IDC_STATIC_CODE, strCode);
	SetDlgItemText(IDC_STATIC_POS, strPos);
	SetDlgItemText(IDC_STATIC_TYPE_INFO, strTypeInfo);

	// 이미지 갱신
	CImage typeImg;
	CString strFolderPath, strSearchPath;
	strFolderPath.Format(_T("%s\\03_type\\%s\\%d"), m_strRootPath, (LPCTSTR)data.m_char, data.m_type);
	strSearchPath.Format(_T("%s\\*.png"), (LPCTSTR)strFolderPath);
	CFileFind finder;
	if (finder.FindFile(strSearchPath)) {
		finder.FindNextFile();
		if (typeImg.Load(finder.GetFilePath()) == S_OK) {
			CWnd* pWnd = GetDlgItem(IDC_PIC_TYPE);
			if (pWnd) {
				CDC* pDC = pWnd->GetDC();
				CRect rect; pWnd->GetClientRect(&rect);
				pDC->FillSolidRect(rect, RGB(255, 255, 255));
				typeImg.Draw(pDC->m_hDC, rect);
				pWnd->ReleaseDC(pDC);
			}
		}
	}

	// 리스트 갱신
	UpdateComponentList();

	*pResult = 0;
}

// =================================================================
// [평가 3] 전체 통계 및 페이지 통계 표시 함수
// =================================================================
void CGomunseoDBDlg::UpdateInfoLabel()
{
	// 변수 초기화
	int bookTotalCount = m_TypeDB.m_nChar;
	set<CString> bookCodeSet;
	set<CString> bookTypeSet;
	int maxSheet = m_TypeDB.m_nSheet;

	int pageTotalCount = 0;
	set<CString> pageCodeSet;
	set<CString> pageTypeSet;

	for (const auto& data : m_TypeDB.m_Chars)
	{
		// 책 전체 집계
		bookCodeSet.insert(data.m_char);
		CString strTypeKey;
		strTypeKey.Format(_T("%s_%d"), (LPCTSTR)data.m_char, data.m_type);
		bookTypeSet.insert(strTypeKey);

		// 현재 페이지 집계
		if (data.m_sheet == m_nCurrentSheet)
		{
			pageTotalCount++;
			pageCodeSet.insert(data.m_char);
			pageTypeSet.insert(strTypeKey);
		}
	}

	// 화면 표시
	CString strVal;
	strVal.Format(_T("%d / %d 장"), m_nCurrentSheet, maxSheet);
	SetDlgItemText(IDC_STATIC_SHEET_INFO, strVal);

	// 책 전체
	strVal.Format(_T("%d 개"), bookTotalCount); SetDlgItemText(IDC_STATIC_BOOK_TOTAL, strVal);
	strVal.Format(_T("%d 종"), (int)bookCodeSet.size()); SetDlgItemText(IDC_STATIC_BOOK_JONG, strVal);
	strVal.Format(_T("%d 개"), (int)bookTypeSet.size()); SetDlgItemText(IDC_STATIC_BOOK_HWAL, strVal);

	// 장 내
	strVal.Format(_T("%d 개"), pageTotalCount); SetDlgItemText(IDC_STATIC_PAGE_TOTAL, strVal);
	strVal.Format(_T("%d 종"), (int)pageCodeSet.size()); SetDlgItemText(IDC_STATIC_PAGE_JONG, strVal);
	strVal.Format(_T("%d 개"), (int)pageTypeSet.size()); SetDlgItemText(IDC_STATIC_PAGE_HWAL, strVal);
}

// =================================================================
// [평가 9] 구성 글자 리스트 업데이트 (같은 Type 목록)
// =================================================================
void CGomunseoDBDlg::UpdateComponentList()
{
	if (m_nSelectedIdx == -1) return;

	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_COMPONENTS);
	if (!pList) return;
	pList->DeleteAllItems();

	SCharInfo& currentData = m_TypeDB.m_Chars[m_nSelectedIdx];

	int row = 0;
	for (int i = 0; i < m_TypeDB.m_Chars.size(); i++)
	{
		// [수정 전] 코드도 같고, 타입도 같아야 함
		// if (m_TypeDB.m_Chars[i].m_char == currentData.m_char &&
		// 	   m_TypeDB.m_Chars[i].m_type == currentData.m_type)

		// [수정 후] ★ Type 상관없이 코드(글자)만 같으면 다 보여줘라!
		if (m_TypeDB.m_Chars[i].m_char == currentData.m_char)
		{
			CString strSheet, strLine, strOrder;
			strSheet.Format(_T("%d"), m_TypeDB.m_Chars[i].m_sheet);
			strLine.Format(_T("%d"), m_TypeDB.m_Chars[i].m_line);
			strOrder.Format(_T("%d"), m_TypeDB.m_Chars[i].m_order);

			pList->InsertItem(row, strSheet);
			pList->SetItemText(row, 1, strLine);
			pList->SetItemText(row, 2, strOrder);
			pList->SetItemText(row, 3, m_TypeDB.m_Chars[i].m_char);

			// 실제 인덱스 숨겨두기 (클릭 시 이미지 띄우기 위해)
			pList->SetItemData(row, i);
			row++;
		}
	}
}

// =================================================================
// [평가 10] 리스트 클릭 시 이미지 보여주기
// =================================================================
void CGomunseoDBDlg::OnNMClickListComponents(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = (reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR));

	int nItem = pNMItemActivate->iItem;
	if (nItem != -1)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_COMPONENTS);
		int realIdx = (int)pList->GetItemData(nItem);

		SCharInfo& data = m_TypeDB.m_Chars[realIdx];

		// 이미지 로드 (아까 03_type 로직 재사용)
		CImage typeImg;
		CString strFolderPath, strSearchPath;
		strFolderPath.Format(_T("%s\\03_type\\%s\\%d"), m_strRootPath, (LPCTSTR)data.m_char, data.m_type);
		strSearchPath.Format(_T("%s\\*.png"), (LPCTSTR)strFolderPath);

		CFileFind finder;
		if (finder.FindFile(strSearchPath)) {
			finder.FindNextFile();
			if (typeImg.Load(finder.GetFilePath()) == S_OK) {
				CWnd* pWnd = GetDlgItem(IDC_PIC_SELECTED_COMP); // 여기 Picture Control ID 주의!
				if (pWnd) {
					CDC* pDC = pWnd->GetDC();
					CRect rect; pWnd->GetClientRect(&rect);
					pDC->FillSolidRect(rect, RGB(255, 255, 255));
					typeImg.Draw(pDC->m_hDC, rect);
					pWnd->ReleaseDC(pDC);
				}
			}
		}
	}
	*pResult = 0;
}
void CGomunseoDBDlg::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

// GomunseoDBDlg.cpp 맨 아래 추가

void CGomunseoDBDlg::OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = (LPNMUPDOWN)pNMHDR;

	// 책이 로드되지 않았으면 무시
	if (m_strRootPath.IsEmpty() || m_TypeDB.m_nSheet == 0) {
		*pResult = 0;
		return;
	}

	// 화살표 방향에 따라 페이지 증감
	// (iDelta < 0 이면 위쪽 화살표 -> 다음 페이지)
	// (iDelta > 0 이면 아래쪽 화살표 -> 이전 페이지)
	if (pNMUpDown->iDelta < 0)
	{
		// [다음 페이지]
		if (m_nCurrentSheet < m_TypeDB.m_nSheet) {
			m_nCurrentSheet++;
		}
	}
	else
	{
		// [이전 페이지]
		if (m_nCurrentSheet > 1) {
			m_nCurrentSheet--;
		}
	}

	// 화면 갱신
	UpdateInfoLabel();  // "2 / 3 장" 텍스트 업데이트
	Invalidate();       // 이미지 새로 그리기
	UpdateWindow();

	*pResult = 0;
}

// GomunseoDBDlg.cpp 맨 아래 추가

void CGomunseoDBDlg::OnBnClickedBtnOpenFolder()
{
	// 1. 폴더 선택 대화상자 띄우기 (교수님 평가항목 2번)
	CFolderPickerDialog dlg;
	if (dlg.DoModal() != IDOK) return;

	m_strRootPath = dlg.GetPathName(); // 전체 경로 저장 (예: C:\Project\GomunseoDB)

	// ==========================================
	// [추가] 폴더 이름만 잘라서 콤보 박스에 넣기 (폼생폼사!)
	// ==========================================
	int idx = m_strRootPath.ReverseFind('\\'); // 뒤에서부터 '\' 위치 찾기
	CString folderName = m_strRootPath.Mid(idx + 1); // 그 뒤의 글자(폴더명)만 추출

	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_BOOK);
	if (pCombo) {
		pCombo->ResetContent();       // 기존 내용 지우기
		pCombo->AddString(folderName); // "GomunseoDB" 또는 "월인천강지곡" 추가
		pCombo->SetCurSel(0);         // 첫 번째 항목 선택
	}

	// 2. 모래시계 커서 (평가 11번)
	BeginWaitCursor();

	// 3. 데이터 읽기
	CString csvPath;
	csvPath.Format(_T("%s\\typeDB.csv"), (LPCTSTR)m_strRootPath); // LPCTSTR 꼭 붙이기!

	if (m_TypeDB.ReadCSVFile(csvPath))
	{
		// 초기화
		m_nCurrentSheet = 1;
		m_nSelectedIdx = -1;
		m_sameCodeList.clear();

		// 1행 1번 글자 자동 선택 (평가 5번)
		for (int i = 0; i < m_TypeDB.m_Chars.size(); i++) {
			if (m_TypeDB.m_Chars[i].m_sheet == 1 &&
				m_TypeDB.m_Chars[i].m_line == 1 &&
				m_TypeDB.m_Chars[i].m_order == 1)
			{
				m_nSelectedIdx = i;

				// 스핀 컨트롤용 리스트 구성
				m_nTypeViewIdx = 0;
				for (int k = 0; k < m_TypeDB.m_Chars.size(); k++) {
					if (m_TypeDB.m_Chars[k].m_char == m_TypeDB.m_Chars[i].m_char) {
						m_sameCodeList.push_back(k);
						if (k == i) m_nTypeViewIdx = (int)m_sameCodeList.size() - 1;
					}
				}
				break;
			}
		}

		// 화면 갱신
		UpdateInfoLabel();
		UpdateComponentList();
		Invalidate();

		EndWaitCursor();
		// 성공 메시지는 귀찮으면 빼셔도 됩니다.
		// AfxMessageBox(_T("책을 불러왔습니다.")); 
	}
	else
	{
		EndWaitCursor();
		AfxMessageBox(_T("폴더 내에 typeDB.csv 파일이 없습니다."));
	}
}