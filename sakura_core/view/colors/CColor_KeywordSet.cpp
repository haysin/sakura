#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_KeywordSet.h"
#include <limits>
#include "mem/CNativeW.h"
#include "charset/charcode.h"

/** �����𕪗ނ���B
	@retval 0 �L�[���[�h�Ɏg���镶���ł͂Ȃ��B
	@retval ���̑��̐� �A�����铯�����̕�������̃L�[���[�h������B
	���Ƃ��΁AABCDEFG�Ƃ��������񂪂���A���ꂼ��̕����� CharType�����Ԃ� 1121133�������Ƃ���ƁAAB�AC�ADE�AFG���L�[���[�h���ɂȂ�B
*/
static int CharType( const wchar_t wch );

/** start�����̌�̋��E�̈ʒu��Ԃ��B
	start���O�̕����͓ǂ܂Ȃ��B��ԑ傫���߂�l�� str.GetLength()�Ɠ������Ȃ�B
*/
static int NextWordBreak( const CStringRef& str, const int start );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�Z�b�g                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColor_KeywordSet::CColor_KeywordSet()
: m_nKeywordIndex(0)
, m_nCOMMENTEND(0)
{
}


// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
bool CColor_KeywordSet::BeginColor(const CStringRef& cStr, int nPos)
{
	if( ! cStr.IsValid() ) {
		return false; // �ǂ��ɂ��ł��Ȃ��B
	}
	const CEditDoc* const pDoc = CEditDoc::GetInstance(0);
	const STypeConfig& doctype = pDoc->m_cDocType.GetDocumentAttribute();
	
	/*
		Summary:
			���݈ʒu����L�[���[�h�𔲂��o���A���̃L�[���[�h���o�^�P��Ȃ�΁A�F��ς���
	*/

	const int charType = CharType( cStr.At( nPos ) );
	if( ! charType ) {
		return false; // ���̕����̓L�[���[�h�Ώە����ł͂Ȃ��B
	}
	if( 0 < nPos && charType == CharType( cStr.At( nPos - 1 ) ) ) {
		return false; // ��̋��E�ł͂Ȃ������B
	}

	const int posNextWordHead = NextWordBreak( cStr, nPos );
	for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; ++i ) {
		if( ! doctype.m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ].m_bDisp ) {
			continue; // �F�ݒ肪��\���Ȃ̂ŃX�L�b�v�B
		}
		const int iKwdSet = doctype.m_nKeyWordSetIdx[i];
		if( iKwdSet == -1 ) {
			continue; // �L�[���[�h�Z�b�g���ݒ肳��Ă��Ȃ��̂ŃX�L�b�v�B
		}
		int posWordEnd = nPos; ///< nPos...posWordEnd���L�[���[�h�B
		int posWordEndCandidate = posNextWordHead; ///< nPos...posWordEndCandidate�̓L�[���[�h���B
		do {
			const int ret = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2( iKwdSet, cStr.GetPtr() + nPos, posWordEndCandidate - nPos );
			if( 0 <= ret ) {
				// �o�^���ꂽ�L�[���[�h�������B
				posWordEnd = posWordEndCandidate;
				if( ret == std::numeric_limits<int>::max() ) {
					// ��蒷���L�[���[�h�����݂���̂ŉ������ă��g���C�B
					continue;
				}
				break;
			} else if( ret == -1 ) {
				// �o�^���ꂽ�L�[���[�h�ł͂Ȃ������B
				break;
			} else if( ret == -2 ) {
				// ����������Ȃ������̂ŉ������ă��g���C�B
				continue;
			} else {
				// �o�^���ꂽ�L�[���[�h�ł͂Ȃ������H
				// CKeyWordSetMgr::SearchKeyWord2()����z��O�̖߂�l�B
				break;
			}
		} while( posWordEndCandidate < cStr.GetLength() && (posWordEndCandidate = NextWordBreak( cStr, posWordEndCandidate )) );

		// nPos...posWordEnd ���L�[���[�h�B
		if( nPos < posWordEnd ) {
			this->m_nCOMMENTEND = posWordEnd;
			this->m_nKeywordIndex = i;
			return true;
		}
	}
	return false;
}

bool CColor_KeywordSet::EndColor(const CStringRef& cStr, int nPos)
{
	return nPos == this->m_nCOMMENTEND;
}


static inline int CharType( const wchar_t wch )
{
	if( ! wch || WCODE::IsBlank( wch ) || WCODE::IsLineDelimiter( wch ) || WCODE::IsControlCode( wch ) ) {
		return 0;
	}
	const int charType = IS_KEYWORD_CHAR( wch ) ? 1 : 2;
	return charType;
}

static int NextWordBreak( const CStringRef& str, const int start )
{
	const int charType = CharType( str.At( start ) );
	int result = start;
	for( ; result < str.GetLength(); result += CNativeW::GetSizeOfChar( str.GetPtr(), str.GetLength(), result ) ) {
		if( charType != CharType( str.At( result ) ) ) {
			break;
		}
	}
	return result;
}