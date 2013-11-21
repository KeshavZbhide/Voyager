// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoUpdateProcess.h"
#include "base\file_util.h"
#include "base\file_path.h"
#include "base\win\shortcut.h"

#include "UIMain.h"
#include "KaminoUpdateStatus.h"
#include "content\public\browser\browser_thread.h"
#include "third_party\libxml\chromium\libxml_utils.h"


#include <windows.h>

namespace KaminoUpdate{
void RecivedResourceFile(FilePath path);
void LastStepOfExecution(bool set_shortcut, std::string &shortcut, bool set_version, std::string &ver);

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length();
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
    return r;
}

wchar_t *s2wsArray(char *s){
	int slen = strlen(s) + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, s, slen, NULL, 0);
	wchar_t *new_str = (wchar_t *)malloc(len);
	for(int i = 0; i<len; i++)
		((char *)new_str)[i] = '\0';
	MultiByteToWideChar(CP_ACP, 0, s, slen, new_str, len);
	return new_str;
}


class KaminoUpdateProcessUrlFetcher : public net::URLFetcherDelegate{
public:	
	net::URLFetcher *fetcher;
	std::string source_url;
	FilePath response_file_path;
	bool is_file;
	base::Callback<void(std::string)> exec_for_raw_response;
	base::Callback<void(FilePath)> exec_for_file;
	static net::URLRequestContextGetter *backup_url_request_getter;
	static std::vector<KaminoUpdateProcessUrlFetcher *> *all_fetches;

	KaminoUpdateProcessUrlFetcher(net::URLRequestContextGetter *getter, std::string &url) : net::URLFetcherDelegate(), 
		source_url(url.c_str()), response_file_path(){
		fetcher = NULL;
		fetcher = net::URLFetcher::Create(0, GURL(source_url), net::URLFetcher::GET, this);	
		if(getter){
			fetcher->SetRequestContext(getter);
			backup_url_request_getter = getter;
		}
		else
			fetcher->SetRequestContext(backup_url_request_getter);
		is_file = false;
		if(all_fetches == NULL)
			all_fetches = new std::vector<KaminoUpdateProcessUrlFetcher *>();
	}
	
	~KaminoUpdateProcessUrlFetcher(){
		if(fetcher)
			delete fetcher;
	}
	
	void Execute(base::Callback<void(std::string)> callback){
		is_file = false;
		exec_for_raw_response = callback;
		fetcher->Start();
	}
	
	void Execute(base::Callback<void(FilePath)> callback, std::string &file_path){
		all_fetches->push_back(this);
		is_file = true;
		exec_for_file = callback;
		response_file_path = FilePath(s2ws(file_path));
		if(!file_util::PathExists(response_file_path.DirName())){
			file_util::CreateDirectory(response_file_path.DirName());
		}
		if(file_util::PathExists(response_file_path)){			
			/*Build A new Path that has .temp extension attached...*/
			//printf("\nPathExisits hence modifying Extension");
			response_file_path = response_file_path.AddExtension(L".temp");
			//wprintf(L"\n New FilePath str is %s", response_file_path.value().c_str());
		}
		fetcher->SaveResponseToFileAtPath(response_file_path, MessageLoop::current()->message_loop_proxy());
		fetcher->Start();
	}

	//This Method Does Not records to all_fetches...
	void ExecuteWithoutRecording(base::Callback<void(FilePath)> callback, std::string &file_path){
		is_file = true;
		exec_for_file = callback;
		response_file_path = FilePath(s2ws(file_path));
		if(!file_util::PathExists(response_file_path.DirName())){
			file_util::CreateDirectory(response_file_path.DirName());
		}
		if(file_util::PathExists(response_file_path)){			
			/*Build A new Path that has .temp extension attached...*/
			//printf("\nPathExisits hence modifying Extension");
			response_file_path = response_file_path.AddExtension(L".temp");
			//wprintf(L"\n New FilePath str is %s", response_file_path.value().c_str());
		}
		fetcher->SaveResponseToFileAtPath(response_file_path, MessageLoop::current()->message_loop_proxy());
		fetcher->Start();
	}
	
	virtual void OnURLFetchComplete(const net::URLFetcher* source) OVERRIDE { 
		if(!source->GetStatus().is_success()){
			//printf("URL Request Failed for source URL %s", source->GetOriginalURL().spec().c_str());
			return;
		}
		if(is_file){
			FilePath path;
			fetcher->GetResponseAsFilePath(true, &path);
			exec_for_file.Run(path);
			for(std::vector<KaminoUpdateProcessUrlFetcher *>::iterator it = all_fetches->begin(); it != all_fetches->end(); it++){
				if(*it == this){
					all_fetches->erase(it);
					//printf("\nRemoved An Entry From Std::Vector");
					if(all_fetches->empty()){
						//printf("\n All Fetches Completed");
						std::string temp;
						LastStepOfExecution(false, temp, false, temp);
					}
					break;
				}						
			}			
		}else{
			std::string response;
			fetcher->GetResponseAsString(&response);
			exec_for_raw_response.Run(response);
		}
		delete this;
	}
	
	virtual void OnURLFetchDownloadProgress(const net::URLFetcher* source, int64 current, int64 total) OVERRIDE { }
	virtual void OnURLFetchDownloadData(const net::URLFetcher* source, scoped_ptr<std::string> download_data) OVERRIDE { }
	virtual bool ShouldSendDownloadData() OVERRIDE { return false;}
	virtual void OnURLFetchUploadProgress(const net::URLFetcher* source, int64 current, int64 total) OVERRIDE { }
};
std::vector<KaminoUpdateProcessUrlFetcher *> * KaminoUpdateProcessUrlFetcher::all_fetches = NULL;
net::URLRequestContextGetter *KaminoUpdateProcessUrlFetcher::backup_url_request_getter = NULL;

void LastStepOfExecution(bool set_shortcut, std::string &shortcut, bool set_version, std::string &ver){
	static std::string shortcut_target;
	static std::string version;
	if(set_shortcut)
		shortcut_target = shortcut;
	else if(set_version)
			version = ver;
		else{
			char current_directory_array[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, current_directory_array);
			for(char *slash = strrchr(current_directory_array, '\\'); (slash - current_directory_array < MAX_PATH); slash++)
				*slash = 0;			
			std::string current_directory(current_directory_array);
			std::string state_path = current_directory + "\\state.xml";
			std::string version_path = current_directory + "\\current_version.txt";
			std::string state_url("https://s3-us-west-2.amazonaws.com/voyjor/"  + version + "/state.xml");
			std::string version_url("https://s3-us-west-2.amazonaws.com/voyjor/current_version.txt");
			KaminoUpdateProcessUrlFetcher *fetch_state = new KaminoUpdateProcessUrlFetcher(NULL, state_url);			
			KaminoUpdateProcessUrlFetcher *fetch_version = new KaminoUpdateProcessUrlFetcher(NULL, version_url);
			fetch_state->ExecuteWithoutRecording(base::Bind(RecivedResourceFile), state_path);
			fetch_version->ExecuteWithoutRecording(base::Bind(RecivedResourceFile), version_path);
			if(!shortcut_target.empty()){
				KaminoUpdateProcess::BuildAllShortCuts(shortcut_target);		
			}
		}
}



KaminoUpdateProcess::KaminoUpdateProcess(){
	wnd_cls.cbClsExtra = 0;
	wnd_cls.cbSize = sizeof(WNDCLASSEX);
	wnd_cls.cbWndExtra = 0;
	wnd_cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd_cls.hInstance = GetModuleHandle(NULL);
	wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.lpfnWndProc = DefWindowProc;
	wnd_cls.lpszMenuName = NULL;
	wnd_cls.lpszClassName = L"KaminoUpdateProcess";
	wnd_cls.lpszMenuName = NULL;
	wnd_cls.style = CS_VREDRAW | CS_HREDRAW;
}

KaminoUpdateProcess::~KaminoUpdateProcess(){
	DestroyWindow(hWnd);
}

void KaminoUpdateProcess::BringFile(std::string &file, std::string &url, base::Callback<void(FilePath)> callback){
	KaminoUpdateProcessUrlFetcher *fetcher = new KaminoUpdateProcessUrlFetcher(url_request_context_getter, url);
	fetcher->Execute(callback, file);
}

void RecivedResourceFile(FilePath path){
	//wprintf(L"\nRecived Resource File %s", path.value().c_str());
	if(path.Extension().compare(L".temp") == 0){
		FilePath new_path = path.RemoveExtension();
		file_util::Delete(new_path, false);
		//printf("\nReplacing File");
		file_util::ReplaceFile(path, new_path);
	}	
}


void KaminoUpdateProcess::BuildCurrentState(){
}

//Checks if We Need to Bring New Pathces Or Executables to rebuild the main executuable directory...
bool KaminoUpdateProcess::ShouldUnderGoAppUpdateProcedure(std::string &possible_version){
	reader_current_state.Load(current_state_xml);
	while(reader_current_state.Read()){
		if(reader_current_state.NodeName().compare("APP_DIR") == 0){
			std::string app_version;
			reader_current_state.NodeAttribute("name", &app_version);
			if(app_version.compare(possible_version) == 0)
				return false;
			else
				return true;
		}
	}
	return true;
}

bool KaminoUpdateProcess::ShouldPatchExecutable(std::string &name, std::string &version){
	reader_current_state.Load(current_state_xml);
	while(reader_current_state.Read()){
		if(reader_current_state.NodeName().compare("executable") == 0){
			std::string current_name;
			std::string current_version;
			reader_current_state.NodeAttribute("name", &current_name);
			if(current_name.compare(name) == 0){
				reader_current_state.NodeAttribute("version", &current_version);
				int current_version_int	= atoi(current_version.c_str());
				int version_int = atoi(version.c_str());
				if((version_int > current_version_int) && ((version_int - current_version_int) == 1))
					return true;
				else 
					return false;
			}			
		}
	}
	return false;	
}


void PatchExecutableCallBack(FilePath path){
	std::wstring new_output_file = path.RemoveExtension().value();
	//wprintf(L"\nRecived Patch File with Path %s", path.value().c_str());
	//wprintf(L"\nExecutable Should be %s", new_output_file.c_str());
	STARTUPINFO info = { sizeof(info) };
	info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	info.hStdOutput =  GetStdHandle(STD_OUTPUT_HANDLE);
	info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	info.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION processInfo;
	std::wstring command(L"-apply ");
	//wprintf(L"\nCommand is %s", command.c_str());
	std::wstring applicant = std::wstring(wcsrchr(new_output_file.c_str(), '\\')+1);
	//wprintf(L"\nApplicant is %s", applicant.c_str());	
	std::wstring patch_file = L" \"" + path.value() + L"\"";
	//wprintf(L"\nPatch File is%s", patch_file.c_str());
	std::wstring output = L" \"" + new_output_file + L"\"";
	//wprintf(L"\nOutPut should be%s", output.c_str());	
	std::wstring full_command_line = L"courgette.exe -apply " + applicant + patch_file + output;
	//wprintf(L"\nFull Command Line is %s\n\n", full_command_line.c_str());
	CreateProcess(L"courgette.exe", (LPWSTR)full_command_line.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);														
}

bool KaminoUpdateProcess::PatchExecutable(std::string &in_dir, std::string &patch_src){
	//printf("\nShould Patch Executable insid Directory[ %s ], with patch_src == %s", in_dir.c_str(), patch_src.c_str());
	std::string patch_file = in_dir + "\\" + std::string(strrchr(patch_src.c_str(), '/')+1);
	BringFile(patch_file, patch_src, base::Bind(PatchExecutableCallBack));
	return true;
}


bool KaminoUpdateProcess::ShouldBringRawExecutable(std::string &name, std::string &version){
	reader_current_state.Load(current_state_xml);
	while(reader_current_state.Read()){
		if(reader_current_state.NodeName().compare("executable") == 0){
			std::string current_name;
			std::string current_version;
			reader_current_state.NodeAttribute("name", &current_name);
			if(current_name.compare(name) == 0){
				reader_current_state.NodeAttribute("version", &current_version);		
				if(current_version.compare(version) == 0)
					return false;
				else
					return true;
			}
		}	
	}
	return true;
}

void KaminoUpdateProcess::BuildAllShortCuts(std::string &target){
	base::win::ShortcutProperties shortcut_props;
	wchar_t user_profil_dir[MAX_PATH];
	DWORD d = MAX_PATH;
	GetUserName(user_profil_dir, &d);
	std::wstring user_name = std::wstring(user_profil_dir);
	FilePath link_target(s2ws(target));	
	shortcut_props.set_target(link_target);
	shortcut_props.set_description(L"browser");
	shortcut_props.set_working_dir(link_target.DirName());

	std::wstring startMenu = std::wstring(L"C:\\Users\\") + 
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs");						
	if(file_util::PathExists(FilePath(startMenu))){
		FilePath link(startMenu + L"\\Voyjor.lnk");
		if(file_util::PathExists(link)){
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_UPDATE_EXISTING);
			//wprintf(L"\nUpdateing New Link %s", link.value().c_str());			
		}
		else{
			//wprintf(L"\nCreated Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
		}
	}
	std::wstring pin_to_taskbar = std::wstring(L"C:\\Users\\") +
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar");	
	if(file_util::PathExists(FilePath(pin_to_taskbar))){
		FilePath link(pin_to_taskbar + L"\\Voyjor.lnk");
		if(file_util::PathExists(link)){
			//wprintf(L"\nUpdated Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_UPDATE_EXISTING);
		}
		//else
			//wprintf(L"\nLink Does Not Exist for path %s", link.value().c_str());
	}
	std::wstring pin_to_startmenu = std::wstring(L"C:\\Users\\") +
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\StartMenu");	
	if(file_util::PathExists(FilePath(pin_to_startmenu))){
		FilePath link(pin_to_startmenu + L"\\Voyjor.lnk");
		if(file_util::PathExists(link)){
			//wprintf(L"\nUpdated Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_UPDATE_EXISTING);		
		}
		//else
			//wprintf(L"\nLink Does Not Exist for path %s", link.value().c_str());
	}	
	std::wstring desktop = std::wstring(L"C:\\Users\\") + user_name + std::wstring(L"\\Desktop");
	if(file_util::PathExists(FilePath(desktop))){
		FilePath link(desktop + L"\\Voyjor.lnk");
		if(file_util::PathExists(link)){
			//wprintf(L"\nUpdate Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_UPDATE_EXISTING);		
		}
		else{
			//wprintf(L"\nCreate Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
		}
	}
	std::wstring quick_launch = std::wstring(L"C:\\Users\\") + 
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch");	
	if(file_util::PathExists(FilePath(quick_launch))){
		FilePath link(quick_launch + L"\\Voyjor.lnk");
		if(file_util::PathExists(link)){
			//wprintf(L"\nUpdate Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_UPDATE_EXISTING);		
		}
		else{
			//wprintf(L"\nCreate Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
		}
	}
}


void KaminoUpdateProcess::StartPatchingAndUpdatingBinaryExecutables(std::string stateXml){
	//printf("\nNow Started Briging And patching New Executables");
	XmlReader reader;
	reader.Load(stateXml);
	bool inside_dir = false;
	char current_directory_array[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, current_directory_array);
	for(char *slash = strrchr(current_directory_array, '\\'); (slash - current_directory_array < MAX_PATH); slash++)
		*slash = '\0';
	std::string current_directory(current_directory_array);
	while(reader.Read()){
		if(reader.NodeName().compare("APP_DIR") == 0){
			inside_dir = !inside_dir;
			if(inside_dir){
				//Enterd A Directory
				std::string dir_name;
				reader.NodeAttribute("name", &dir_name);
				if(ShouldUnderGoAppUpdateProcedure(dir_name)){	
					current_directory += "\\" + dir_name;
					file_util::CreateDirectory(FilePath(s2ws(current_directory)));
				}
				else 
					break;
			}
			else
				current_directory = std::string(current_directory_array);
		}
		else
			if(reader.NodeName().compare("executable") == 0){
				std::string attr;
				std::string version;
				std::string shortcut_linking;
				reader.NodeAttribute("name", &attr);
				reader.NodeAttribute("version", &version);
				reader.NodeAttribute("link", &shortcut_linking);

				if((!shortcut_linking.empty()) && (shortcut_linking.compare("ME") == 0)){
					//Setup New ShortCuts for this Executable....
					std::string target = current_directory + "\\" + attr;
					std::string temp;
					LastStepOfExecution(true, target, false, temp); 
				}
				
				if(ShouldPatchExecutable(attr, version)){
					std::string patch_src;
					reader.NodeAttribute("patch", &patch_src);
					PatchExecutable(current_directory, patch_src);
				}
				else if(ShouldBringRawExecutable(attr, version)){ 
						//printf("\nShould Bring Raw Executable %s From S3", attr.c_str());
						std::string source;
						reader.NodeAttribute("src", &source);
						std::string output = current_directory + "\\" + attr;
						BringFile(output, source, base::Bind(RecivedResourceFile));
					}
					else{
						//This is where we copy the executable from previous directory and past it in the new directory

						char dr[MAX_PATH];
						GetCurrentDirectoryA(MAX_PATH, dr);
						//printf("\nShould Move Executable %s From Previous Folder To New", attr.c_str());
						std::wstring new_location = s2ws(current_directory + "\\" + attr);
						std::wstring old_location = s2ws(std::string(dr) + "\\" + attr);
						//wprintf(L"\nShould Move %s to %s", old_location.c_str(), new_location.c_str());
						file_util::CopyFileW(FilePath(old_location), FilePath(new_location));
					}
				
			}
	}
}



bool KaminoUpdateProcess::ShouldBringFile(std::string &version, std::string &file){
	reader_current_state.Load(current_state_xml);
	std::string content;
	char current_directory_array[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, current_directory_array);
	//Setup current_directory to parent directory..
	for(char *slash = strrchr(current_directory_array, '\\'); (slash - current_directory_array < MAX_PATH); slash++)
		*slash = 0;
	std::string current_directory(current_directory_array);
	bool inside_dir = false;
	while(reader_current_state.Read()){
		if(reader_current_state.NodeName().compare("DIR") == 0){
			inside_dir = !inside_dir;
			if(inside_dir){
				//Enterd A Directory
				std::string dir_name;
				reader_current_state.NodeAttribute("name", &dir_name);
				current_directory += "\\" + dir_name;
			}
			else
				current_directory = std::string(current_directory_array);
		}
		else
			if(reader_current_state.NodeName().compare("file") == 0){
				std::string attr;
				std::string version_on_system;
				reader_current_state.NodeAttribute("name", &attr);
				std::string file_on_system = current_directory + "\\" + attr;
				reader_current_state.NodeAttribute("version", &version_on_system);
				if(file_on_system.compare(file) == 0){
					if(version_on_system.compare(version) == 0)
						return false;
					else
						return true;
				}
			}
	}
	return true;
}

void KaminoUpdateProcess::InitialExecution2(std::string stateXml){
	//printf("\n[ %s ]", stateXml.c_str());
	XmlReader reader;
	reader.Load(stateXml);
	char current_directory_array[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, current_directory_array);
	for(char *slash = strrchr(current_directory_array, '\\'); (slash - current_directory_array < MAX_PATH); slash++)
		*slash = 0;
	std::string current_directory(current_directory_array);
	bool inside_dir = false;
	while(reader.Read()){
		if(reader.NodeName().compare("DIR") == 0){
			inside_dir = !inside_dir;
			if(inside_dir){
				//Enterd A Directory
				std::string dir_name;
				reader.NodeAttribute("name", &dir_name);
				current_directory += "\\" + dir_name;
			}
			else
				current_directory = std::string(current_directory_array);
		}
		else
			if(reader.NodeName().compare("file") == 0){
				std::string attr;
				std::string version;
				reader.NodeAttribute("name", &attr);
				std::string file = current_directory + "\\" + attr;
				reader.NodeAttribute("version", &version);
				if(ShouldBringFile(version, file)){
					std::string src;
					reader.NodeAttribute("src", &src);
					//printf("\nAbout to Bring File with path %s", file.c_str());
					BringFile(file, src, base::Bind(RecivedResourceFile)); 
				}
			}
	}
	StartPatchingAndUpdatingBinaryExecutables(stateXml);
}

void KaminoUpdateProcess::InitialExecution(std::string version){
	std::string temp;
	LastStepOfExecution(false, temp, true, version);
	std::string main_url = "https://s3-us-west-2.amazonaws.com/voyjor/"  + version + "/state.xml";					
	KaminoUpdateProcessUrlFetcher *fetch = new KaminoUpdateProcessUrlFetcher(url_request_context_getter, main_url);
	fetch->Execute(base::Bind(&KaminoUpdateProcess::InitialExecution2, base::Unretained(this)));
}

void KaminoUpdateProcess::Start(){
	HWND is_update_running = NULL;
	if(FindWindow(L"KaminoUpdateProcess", L"KaminoUpdateProcessWindow") == NULL){
		RegisterClassEx(&wnd_cls);
		hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, wnd_cls.lpszClassName, L"KaminoUpdateProcessWindow", WS_OVERLAPPEDWINDOW, 
			-1, -1, 0, 0, NULL, NULL, wnd_cls.hInstance, NULL);
#ifdef UI_DEBUG_LOG
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
#endif	
		if(!url_request_context_getter)
			url_request_context_getter = new content::KaminoURLRequestContextGetter(
				content::BrowserThread::UnsafeGetMessageLoopForThread(content::BrowserThread::IO),
				content::BrowserThread::UnsafeGetMessageLoopForThread(content::BrowserThread::FILE));
		file_util::ReadFileToString(FilePath(L"..\\state.xml"), &current_state_xml); 
		KaminoUpdateStatus *status = new KaminoUpdateStatus(url_request_context_getter);
		status->Execute(base::Bind(&KaminoUpdateProcess::InitialExecution, base::Unretained(this)));
		UI_LOG(0, "Executing MessageLoop in Update Process")
	}
}

}
