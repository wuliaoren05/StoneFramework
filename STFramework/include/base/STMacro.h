#ifndef ST_MACRO_H
#define ST_MACRO_H

#if !defined(ST_PUB_BASE)
	#define ST_PUB_BASE
#endif

#if !defined(ST_IRV_DEF_OFF)
	#define iRetValue iRet
#endif

#if defined(ST_CMPL_SELF)
	#include "STCompile.h"
#else
	#include "STpub/STCompile.h"
#endif

#if defined(AIXV3) && defined(__64BIT__)
	#define ST_SQL_INT64       "long"
	#define ST_SQL_INT64_TYPE  long
#else
	#define ST_SQL_INT64       "double"
	#define ST_SQL_INT64_TYPE  double
#endif

#if !defined(__FUNCTION__)
	#if defined(LINUX) || defined(HPUX)
		#if defined(DEBUG_MODE)
			#define __FUNCTION__ __func__
			#define ST_FUNC_INFO(szInfo) (CStdStr("ST::") + __FUNCTION__ + szInfo).c_str()
		#else
			#define __FUNCTION__ "function"
		#endif
	#elif defined(SOLARIS)
		#define __FUNCTION__ __func__
		#define ST_FUNC_INFO(szInfo) "ST::function" szInfo
	#else
		#define __FUNCTION__ "function"
	#endif
#endif

#if !defined(ST_FUNC_INFO)
	#define ST_FUNC_INFO(szInfo) "ST::" __FUNCTION__  szInfo

#endif

#if defined(SINUX)
	#define ST_DLL_TAIL "dll"
#elif defined(HPUX)
	#define ST_DLL_TAIL "sl"
#else
	#define ST_DLL_TAIL "so"
#endif

#define ST_CFG_MOD "common"

#if defined(__cplusplus)
	#define ST_EXTERN_C 		extern "C"
	#define ST_EXTERN_C_BGN	extern "C" {
	#define ST_EXTERN_C_END	}
#else
	#define ST_EXTERN_C
	#define ST_EXTERN_C_BGN
	#define ST_EXTERN_C_END
#endif

#define BS_LOG NAMESPACE_BILLING40_LOG
#define BS_FRM NAMESPACE_BILLING40_FRAME

#define CStdStr AISTD string

#define ST_PARA_NULL		//空参数

#define ST_MIN_INT16 (0x8000)
#define ST_MAX_INT16 (0x7FFF)
#define ST_MIN_INT32 (0x80000000)
#define ST_MAX_INT32 (0x7FFFFFFF)
#define ST_MIN_INT64 I64(0x8000000000000000)
#define ST_MAX_INT64 I64(0x7FFFFFFFFFFFFFFF)
#define ST_MSG_CHAR_SIZE  8192
#define ST_ERR_CHAR_SIZE  1204
#define ST_BIT_POS(n) (I64(0x01) << (n-1))
#define ST_ENUM_BIT_POS(n) (0x01 << (n-1))

#define ST_DB	(*pSession->get_dbConn())

#define ST_DEL_PTR(pObj) if(pObj!=NULL){delete pObj; pObj=NULL;}
#define ST_DEL_PTR_ARR(pObjArr) if(pObjArr!=NULL){delete[] pObjArr; pObjArr=NULL;}

#define ST_MIN(a, b) ((a)>(b)?(b):(a))
#define ST_MAX(a, b) ((a)<(b)?(b):(a))
#define ST_MID(x, a, b) (x<a? a :(x>b? b:x))
#define ST_LIST_FIND(listObj, cFindObj)															\
	(AISTD find(listObj.begin(), listObj.end(), cFindObj) != listObj.end())

#if defined(ST_MAP_ORIG)	//是否使用原始map的封装模式
	#define ST_MAP_VAL(mapIn, cKey) mapIn[cKey]
#else
	#define ST_MAP_VAL(mapIn, cKey) STPub::get_mapVal(mapIn, cKey)
#endif

#if defined(ST_VER_SHCM)
	#define ST_SCHEMA_XK "AICBS"
	#define ST_SCHEMA_ZG "AICBS"
	#define ST_SCHEMA_ZC "AICBS"
#else
	#define ST_SCHEMA_XK "ZC"
	#define ST_SCHEMA_ZG "ZG"
	#define ST_SCHEMA_ZC "ZC"
#endif

#define ST_SCHEMA_FUNC_XK(strTbleName) (CStdStr(ST_SCHEMA_XK ".") + strTbleName)
#define ST_SCHEMA_FUNC_ZG(strTbleName) (CStdStr(ST_SCHEMA_ZG ".") + strTbleName)
#define ST_SCHEMA_FUNC_ZC(strTbleName) (CStdStr(ST_SCHEMA_ZC ".") + strTbleName)

#define ST_RV_SUCC  0			//正常返回值; 等价于ST_RV_SUCC
#define ST_RV_999	999			//DBM正常返回值
#define ST_RV_FAIL -1			//出错返回值; 等价于OBD_FAIL
#define ST_RV_GOON  -1001		//用于使程序有错误但可继续运行的返回值
#define ST_RV_EMPTY -1002		//用于查询结果为空的函数调用失败返回值

#define ST_TBL_DUMMY	"ST_DUMMY"	//虚拟空表,分表组件转换结果如为ST_DUMMY，则表示丢弃数据.

#define ST_FMSG(szFormat, ...) STPub::get_formatMsg(szFormat, ## __VA_ARGS__).c_str()

//文件名称与代码行号
#define ST_CUR_POS 		("{" __FILE__ "@" +  itoa(__LINE__) + "}")
//编译日期与变异时间
#define ST_ASM_DTM 		("{" __DATE__ "_"  __TIME__ "}")
//CStdStr 相加操作符宏
#define ST_STR_PLUS 		STPub::plus_str

#define ST_LOG_WRAP(inLogInfo) (inLogInfo + ST_CUR_POS)
#define ST_LOG_WRAP_SZ(inLogInfo) (inLogInfo + ("; " + ST_CUR_POS)).c_str()

#define ST_CLR_ERR_MSG																				\
	cErrorMsg.set_errorCode(0);																		\
	cErrorMsg.set_errorMsg("");																		\
	cErrorMsg.set_hint("")

#define ST_SET_ERR_MSG(strErrMsg)																	\
	cErrorMsg.set_hint(strErrMsg);																	\
	cErrorMsg.set_errorMsg(strErrMsg)																\

/////////////////////////////////////////////////////////////////////////////////////////////////////
//以下宏用于跟踪程序运行流程。
#if defined(ST_LOG_ORIG)	//原始日志形式,不做任何封装,以确认日志封装是否有性能问题
	#define ST_IF(expr)	if(expr)
	#define ST_ELIF(expr)	else if(expr)
	#define ST_ELSE		else
	#define ST_WHILE(expr)	 while(expr)
	#define ST_SWITCH(expr) switch(expr)

	#define ST_HERE
	#define ST_ENTER_FUNC
	#define ST_LEAVE_FUNC
	#define ST_BREAK		break
	#define ST_CONTINUE	continue
	#define ST_RET_VOID	return

	#define ST_FOR(stmt1, expr, stmt3)																\
		for(stmt1; (expr); stmt3)
	#define ST_FOR2(stmt10, stmt11, expr, stmt3)													\
		for(stmt10, stmt11; (expr); stmt3)
	#define ST_FOR3(stmt10, stmt11, stmt12, expr, stmt3)											\
		for(stmt10, stmt11, stmt12; (expr); stmt3)

	//注意：以下ST_RET宏需要用一个语句实现，因为调用者会这么用"if(bVal) ST_RET_XXXX;"
	#define ST_RET_OBJ(cRetObjIn)																	\
		return cRetObjIn
	#define ST_RET_PTR(pRetPtrIn)																	\
		return pRetPtrIn
	#define ST_RET_VAL(cRetValIn)																	\
		return cRetValIn

	#define ST_RET_SUCC																			\
		return ST_RV_SUCC

	#define ST_RET_FAIL																			\
		return ST_RV_FAIL

	//IRV means iRetValue
	#define ST_RET_IRV																				\
		ST_RET_INT(iRetValue)

	#define ST_RET_INT(iRetValueIn)																\
		return iRetValueIn

	//添加else，什么都不做，强制调用者添加分号
	#define ST_RET_IRV_IF_FAIL																		\
		if(iRetValue != ST_RV_SUCC) {																\
			return iRetValue;																		\
		} else

	//SEM means Set Error Msg
	#define ST_RET_FAIL_AFT_SEM(strErrMsg)															\
		ST_RET_INT_AFT_SEM(ST_RV_FAIL, strErrMsg)

	//SEM means Set Error Msg
	#define ST_RET_INT_AFT_SEM(iReValueIn, strErrMsg)										 		\
		return (																					\
			cErrorMsg.set_hint(strErrMsg), cErrorMsg.set_errorMsg(strErrMsg),						\
			iReValueIn																				\
		)
#elif defined(ST_LOG_SMART)	//智能日志形式,避免判断是否打日志时多层函数调用
	#define ST_IF(expr)	if((expr) && (!g_bTraceAble || (ST_LOG_TRACE("    if_entered"), true)))
	#define ST_ELIF(expr)	else if((expr) 															\
		&& (!g_bTraceAble || (ST_LOG_TRACE("    else_if_entered"), true)))
	#define ST_ELSE		else if(!g_bTraceAble || (ST_LOG_TRACE("    else_entered"), true))
	#define ST_WHILE(expr)	while((expr)															\
		&& (!g_bTraceAble || (ST_LOG_TRACE("    while_run_once"), true)))
	#define ST_SWITCH(expr) switch((ST_LOG_TRACE_VAL("    switch_value", expr), expr))

	#define ST_HERE		if(g_bTraceAble) ST_LOG_TRACE("    here_it_goes")
	#define ST_ENTER_FUNC	if(g_bTraceAble) ST_LOG_TRACE(ST_FUNC_INFO(" enter_func"))
	#define ST_LEAVE_FUNC	if(g_bTraceAble) ST_LOG_TRACE(ST_FUNC_INFO(" leave_func"))
	#define ST_BREAK		if(!g_bTraceAble || (ST_LOG_TRACE("    break_here"), true)) break
	#define ST_CONTINUE	if(!g_bTraceAble || (ST_LOG_TRACE("    continue_here"), true)) continue
	#define ST_RET_VOID	if(!g_bTraceAble 														\
		|| (ST_LOG_TRACE(ST_FUNC_INFO(" return_void"),true))) return; else return

	#define ST_FOR(stmt1, expr, stmt3)																\
		for(stmt1; ((expr) && (!g_bTraceAble || (ST_LOG_TRACE("    for_run_once"),true))); stmt3)
	#define ST_FOR2(stmt10, stmt11, expr, stmt3)													\
		for(stmt10, stmt11; 																		\
			((expr) && (!g_bTraceAble || (ST_LOG_TRACE("    for_run_once"),true))); stmt3)
	#define ST_FOR3(stmt10, stmt11, stmt12, expr, stmt3)											\
		for(stmt10, stmt11, stmt12;																	\
			((expr) && (!g_bTraceAble || (ST_LOG_TRACE("    for_run_once"),true))); stmt3)

	//注意：以下ST_RET宏需要用一个语句实现，因为调用者会这么用"if(bVal) ST_RET_XXXX;"
	#define ST_RET_OBJ(cRetObjIn)																	\
		return (																					\
			ST_LOG_TRACE_VAL(ST_FUNC_INFO(" return_obj"), (uint64)(void*)&cRetObjIn),				\
			cRetObjIn																				\
		)

	#define ST_RET_PTR(pRetPtrIn)																	\
		return (																					\
			ST_LOG_TRACE_VAL(ST_FUNC_INFO(" return_ptr"), (uint64)(void*)pRetPtrIn),				\
			pRetPtrIn																				\
		)

	#define ST_RET_VAL(cRetValIn)																	\
		return (																					\
			ST_LOG_TRACE_VAL(ST_FUNC_INFO(" return_val"), cRetValIn),								\
			cRetValIn																				\
		)

	#define ST_RET_SUCC																			\
		return ( 																					\
			ST_LOG_TRACE(ST_FUNC_INFO(" return_succ")),											\
			ST_RV_SUCC																				\
		)

	#define ST_RET_FAIL																			\
		return (																					\
			ST_LOG_WARN(ST_FUNC_INFO(" return_fail")),											\
			ST_RV_FAIL																				\
		)

	//IRV means iRetValue
	#define ST_RET_IRV																				\
		ST_RET_INT(iRetValue)

	#define ST_RET_INT(iRetValueIn)																\
		return (iRetValueIn == ST_RV_SUCC) ? (														\
			ST_LOG_TRACE(ST_FUNC_INFO(" return_succ")),											\
			ST_RV_SUCC																				\
		) : (																						\
			ST_LOG_WARN_VAL(ST_FUNC_INFO(" return_value"), (int64)iRetValueIn),					\
			iRetValueIn																				\
		)

	//添加else，什么都不做，强制调用者添加分号
	#define ST_RET_IRV_IF_FAIL																		\
		if(iRetValue != ST_RV_SUCC) {																\
			ST_LOG_WARN_FMSG(ST_FUNC_INFO(" return_error(%d)"), iRetValue);						\
			return iRetValue;																		\
		} else

	//SEM means Set Error Msg
	#define ST_RET_FAIL_AFT_SEM(strErrMsg)															\
		ST_RET_INT_AFT_SEM(ST_RV_FAIL, strErrMsg)

	//SEM means Set Error Msg
	#define ST_RET_INT_AFT_SEM(iReValueIn, strErrMsg)										 		\
		return (																					\
			cErrorMsg.set_hint(strErrMsg), cErrorMsg.set_errorMsg(strErrMsg),						\
			(ST_LOG_WARN_VAL (ST_FUNC_INFO(" error happen"), cErrorMsg)),							\
			(ST_LOG_WARN_FMSG(ST_FUNC_INFO(" return_error(%d)"), iReValueIn)),					\
			iReValueIn																				\
		)
#else
	#define ST_IF(expr)	if((expr) && (ST_LOG_TRACE("    if_entered"), true))
	#define ST_ELIF(expr)	else if((expr) && (ST_LOG_TRACE("    else_if_entered"), true))
	#define ST_ELSE		else if((ST_LOG_TRACE("    else_entered"), true))
	#define ST_WHILE(expr)	while((expr) && (ST_LOG_TRACE("    while_run_once"), true))
	#define ST_SWITCH(expr) switch((ST_LOG_TRACE_VAL("    switch_value", expr), expr))

	#define ST_HERE		ST_LOG_TRACE("    here_it_goes")
	#define ST_ENTER_FUNC	ST_LOG_TRACE(ST_FUNC_INFO(" enter_func"))
	#define ST_LEAVE_FUNC	ST_LOG_TRACE(ST_FUNC_INFO(" leave_func"))
	#define ST_BREAK		if((ST_LOG_TRACE("    break_here"), true)) break
	#define ST_CONTINUE	if((ST_LOG_TRACE("    continue_here"), true)) continue
	#define ST_RET_VOID	if((ST_LOG_TRACE(ST_FUNC_INFO(" return_void"),true))) return; else return

	#define ST_FOR(stmt1, expr, stmt3)																\
		for(stmt1; ((expr) && (ST_LOG_TRACE("    for_run_once"),true)); stmt3)
	#define ST_FOR2(stmt10, stmt11, expr, stmt3)													\
		for(stmt10, stmt11; ((expr) && (ST_LOG_TRACE("    for_run_once"),true)); stmt3)
	#define ST_FOR3(stmt10, stmt11, stmt12, expr, stmt3)											\
		for(stmt10, stmt11, stmt12; ((expr) && (ST_LOG_TRACE("    for_run_once"),true)); stmt3)

	//注意：以下ST_RET宏需要用一个语句实现，因为调用者会这么用"if(bVal) ST_RET_XXXX;"
	#define ST_RET_OBJ(cRetObjIn)																	\
		return (																					\
			ST_LOG_TRACE_VAL(ST_FUNC_INFO(" return_obj"), (uint64)(void*)&cRetObjIn),				\
			cRetObjIn																				\
		)

	#define ST_RET_PTR(pRetPtrIn)																	\
		return (																					\
			ST_LOG_TRACE_VAL(ST_FUNC_INFO(" return_ptr"), (uint64)(void*)pRetPtrIn),				\
			pRetPtrIn																				\
		)

	#define ST_RET_VAL(cRetValIn)																	\
		return (																					\
			ST_LOG_TRACE_VAL(ST_FUNC_INFO(" return_val"), cRetValIn),								\
			cRetValIn																				\
		)

	#define ST_RET_SUCC																			\
		return ( 																					\
			ST_LOG_TRACE(ST_FUNC_INFO(" return_succ")),											\
			ST_RV_SUCC																				\
		)

	#define ST_RET_FAIL																			\
		return (																					\
			ST_LOG_WARN(ST_FUNC_INFO(" return_fail")),											\
			ST_RV_FAIL																				\
		)

	//IRV means iRetValue
	#define ST_RET_IRV																				\
		ST_RET_INT(iRetValue)

	//IRV 999
	#define ST_RET_999																				\
		ST_RET_INT(ST_RV_999)

	#define ST_RET_INT(iRetValueIn)																\
		return (iRetValueIn == ST_RV_SUCC) ? (														\
			ST_LOG_TRACE(ST_FUNC_INFO(" return_succ")),											\
			ST_RV_SUCC																				\
		) : (																						\
			ST_LOG_WARN_VAL(ST_FUNC_INFO(" return_value"), (int64)iRetValueIn),					\
			iRetValueIn																				\
		)

	//添加else，什么都不做，强制调用者添加分号
	#define ST_RET_IRV_IF_FAIL																		\
		if(iRetValue != ST_RV_SUCC) {																\
			ST_LOG_WARN_FMSG(ST_FUNC_INFO(" return_error(%d)"), iRetValue);						\
			return iRetValue;																		\
		} else

	//SEM means Set Error Msg
	#define ST_RET_FAIL_AFT_SEM(strErrMsg)															\
		ST_RET_INT_AFT_SEM(ST_RV_FAIL, strErrMsg)

	//SEM means Set Error Msg
	#define ST_RET_INT_AFT_SEM(iReValueIn, strErrMsg)										 		\
		return (																					\
			cErrorMsg.set_hint(strErrMsg), cErrorMsg.set_errorMsg(strErrMsg),						\
			(ST_LOG_WARN_VAL (ST_FUNC_INFO(" error happen"), cErrorMsg)),							\
			(ST_LOG_WARN_FMSG(ST_FUNC_INFO(" return_error(%d)"), iReValueIn)),					\
			iReValueIn																				\
		)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
#define ST_FOR_ITER(x, y, z)        for(x::iterator       z=y.begin(); z!=y.end(); ++z)
#define ST_FOR_ITER_CONST(x, y, z)  for(x::const_iterator z=y.begin(); z!=y.end(); ++z)

#define ST_CMPL_INFO(mod) ST_LOG_FATAL(CStdStr("Module_")+ mod + "_compiled_at " +  ST_ASM_DTM)

#define ST_THR_ERR_MSG																				\
	{																		 						\
		ST_LOG_WARN_ERM("ST_throw_errorMsg:", cErrorMsg);					 						\
		throw cErrorMsg;																		 	\
	}

#define ST_THR_ERR_MSG_IF_FAIL																		\
	if(iRetValue != ST_RV_SUCC) {																	\
		ST_LOG_WARN_ERM("ST_throw_errorMsg:", cErrorMsg);					 						\
		throw cErrorMsg;																			\
	}

#if defined(AIXV5) || defined(GCC)
	#define ST_STD_COUNT(itBgn, itEnd, cSepor, iSeporCount)										\
			AISTD count(itBgn,itEnd,cSepor)
#else
	#define ST_STD_COUNT(itBgn, itEnd, cSepor, iSeporCount)										\
			(AISTD count(itBgn, itEnd, cSepor, iSeporCount), iSeporCount)
#endif

#define ST_CAT_ERR_MSG																				\
	catch(CBSErrorMsg & errMsg)	{											 						\
		ST_LOG_WARN_ERM("ST_errorMsg_catched:", cErrorMsg);					 					\
		cErrorMsg = errMsg;													 						\
		iRetValue = ST_RV_FAIL;																	\
	}

#define ST_CAT_OTL_EXC																	  			\
	catch(otl_exception& p)	{													 					\
		AISTD string strTmp =  "错误代码:" + AISTD string(itoa(p.code)) + "\n"						\
				" 错误信息:" + AISTD string((char*)p.msg) + "\n"			 							\
				" 错误语句:" + AISTD string(p.stm_text) + "\n";			   							\
		cErrorMsg.set_errorCode(ST_RV_FAIL);									  					\
		cErrorMsg.set_errorMsg(strTmp);										 						\
		cErrorMsg.set_hint(strTmp);											 						\
		ST_LOG_WARN_ERM("ST_otlException_catched:", cErrorMsg);					 				\
		iRetValue = ST_RV_FAIL;																	\
	}

#define ST_CAT_MDB_EXC																  				\
	catch( CMDBException ex) {												   						\
		char	szMsg[1024];												   						\
		sprintf(szMsg, "MDB exception: code:%d,err:%s,msg:%s",										\
				ex.get_code(), ex.get_err(), ex.get_msg());											\
		cErrorMsg.set_errorCode(ex.get_code());										  				\
		cErrorMsg.set_errorMsg(szMsg);										 						\
		cErrorMsg.set_hint(szMsg);																	\
		ST_LOG_WARN_ERM("ST_mdbException_catched:", cErrorMsg);					 				\
		iRetValue = ST_RV_FAIL;																	\
	}

#define ST_CAT_STD_EXC																				\
	catch (AISTD exception& e) {										  							\
		cErrorMsg.set_errorCode(ST_RV_FAIL);										  				\
		cErrorMsg.set_errorMsg(e.what());											 				\
		cErrorMsg.set_hint(e.what());											 					\
		ST_LOG_WARN_ERM("ST_stdException_catched:", cErrorMsg);						 			\
		iRetValue = ST_RV_FAIL;										 							\
	}

#define ST_CAT_DOT_EXC										  										\
	catch(...){																	  					\
		cErrorMsg.set_errorCode(ST_RV_FAIL);										  				\
		cErrorMsg.set_errorMsg("出现未知异常!");													\
		cErrorMsg.set_hint("出现未知异常!");														\
		ST_LOG_WARN_ERM("ST_unknownException_catched:", cErrorMsg);								\
		iRetValue = ST_RV_FAIL;																	\
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//增加 STPub::get_sysLogType 主要是为了强制设置 sysLogType.
#define ST_LOG_TAIL_PARA iSysLogType, szFile, iLine, szDate, szTime
#define ST_LOG_TAIL_NULL_PARA 0, NULL, 0, NULL, NULL
#define ST_LOG_TAIL_VALS STPub::get_sysLogType(), __FILE__, __LINE__, __DATE__, __TIME__
#define ST_LOG_TAIL_ARGS int32 iSysLogType, const char* szFile, int iLine, const char* szDate, const char* szTime

#define ST_LOG_INIT(szModule, iSysLogType)															\
	if(!STPub::is_sysLogInited()) STPub::init_sysLog(szModule, iSysLogType, __FILE__, __LINE__, __DATE__, __TIME__)

#define ST_LGLVL_TRACE 	-1
#define ST_LGLVL_DEBUG		 0
#define ST_LGLVL_INFO		 1
#define ST_LGLVL_WARN		 2
#define ST_LGLVL_FATAL		 4
#define ST_LGLVL_NONE		 8	//任何日志都不打印
#define ST_LGLVL_DEFT 		16	//日志未初始化默认值

#if defined(ST_COUT_ON)
	#define ST_COUT(zsLogMsg)																	\
		AISTD cout << "AbmTest:" << zsLogMsg << " " << ST_CUR_POS << AISTD endl
	#define ST_COUT_VAL(zsLogMsg, cLogVal)														\
		AISTD cout << "AbmTest:{title:" << zsLogMsg << ", value:" << cLogVal << ST_CUR_POS << AISTD endl
#else
	#define ST_COUT(zsLogMsg)
	#define ST_COUT_VAL(zsLogMsg, cLogVal)
#endif

#define ST_LOG_LEVEL(iLogLevel, szLogInfo)															\
	STPub::write_logMsg(iLogLevel, szLogInfo, ST_LOG_TAIL_VALS)
#define ST_LOG_LEVEL_VAL(iLogLevel, szLogInfo, cLogVal)											\
	STPub::write_logInfo(iLogLevel, szLogInfo, cLogVal,  ST_LOG_TAIL_VALS)
#define ST_LOG_LEVEL_FMSG(iLogLevel, szFormat, ...)												\
	STPub::write_logFmsg(iLogLevel, ST_LOG_TAIL_VALS, szFormat, ## __VA_ARGS__)

#define ST_LOG_TRACE(szLogInfo)															  		\
	ST_LOG_LEVEL(ST_LGLVL_TRACE, szLogInfo)
#define ST_LOG_DEBUG(szLogInfo)															  		\
	ST_LOG_LEVEL(ST_LGLVL_DEBUG, szLogInfo)
#define ST_LOG_INFO(szLogInfo)																  		\
	ST_LOG_LEVEL(ST_LGLVL_INFO, szLogInfo)
#define ST_LOG_WARN(szLogInfo)																  		\
	ST_LOG_LEVEL(ST_LGLVL_WARN, szLogInfo)
#define ST_LOG_FATAL(szLogInfo)															  		\
	ST_LOG_LEVEL(ST_LGLVL_FATAL, szLogInfo)

#define ST_LOG_TRACE_VAL(szLogInfo, cLogVal)												  		\
	ST_LOG_LEVEL_VAL(ST_LGLVL_TRACE, szLogInfo, cLogVal)
#define ST_LOG_DEBUG_VAL(szLogInfo, cLogVal)												  		\
	ST_LOG_LEVEL_VAL(ST_LGLVL_DEBUG, szLogInfo, cLogVal)
#define ST_LOG_INFO_VAL(szLogInfo, cLogVal)												  		\
	ST_LOG_LEVEL_VAL(ST_LGLVL_INFO, szLogInfo, cLogVal)
#define ST_LOG_WARN_VAL(szLogInfo, cLogVal)												  		\
	ST_LOG_LEVEL_VAL(ST_LGLVL_WARN, szLogInfo, cLogVal)
#define ST_LOG_FATAL_VAL(szLogInfo, cLogVal)												  		\
	ST_LOG_LEVEL_VAL(ST_LGLVL_FATAL, szLogInfo, cLogVal)

#define ST_LOG_TRACE_FMSG(szFormat, ...)															\
	ST_LOG_LEVEL_FMSG(ST_LGLVL_TRACE, szFormat, ## __VA_ARGS__)
#define ST_LOG_DEBUG_FMSG(szFormat, ...)															\
	ST_LOG_LEVEL_FMSG(ST_LGLVL_DEBUG, szFormat, ## __VA_ARGS__)
#define ST_LOG_INFO_FMSG(szFormat, ...)															\
	ST_LOG_LEVEL_FMSG(ST_LGLVL_INFO, szFormat, ## __VA_ARGS__)
#define ST_LOG_WARN_FMSG(szFormat, ...)															\
	ST_LOG_LEVEL_FMSG(ST_LGLVL_WARN, szFormat, ## __VA_ARGS__)
#define ST_LOG_FATAL_FMSG(szFormat, ...)															\
	ST_LOG_LEVEL_FMSG(ST_LGLVL_FATAL, szFormat, ## __VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // #define __ACC_CPNT_MACRO__
