

#ifndef TPROJECT_INCLUDED
#define TPROJECT_INCLUDED
#include <set>

#include "tfilepath.h"
#include "tsmartpointer.h"

class ToonzScene;
class TSceneProperties;

#undef DVAPI
#undef DVVAR
#ifdef TOONZLIB_EXPORTS
#define DVAPI DV_EXPORT_API
#define DVVAR DV_EXPORT_VAR
#else
#define DVAPI DV_IMPORT_API
#define DVVAR DV_IMPORT_VAR
#endif

class DVAPI TProject : public TSmartObject
{

	TFilePath m_name, m_path;
	std::vector<string> m_folderNames;
	std::map<string, TFilePath> m_folders;
	std::map<string, bool> m_useScenePathFlags;
	TSceneProperties *m_sprop;

public:
	// default folders names
	static const string Inputs;
	static const string Drawings;
	static const string Scenes;
	static const string Extras;
	static const string Outputs;
	static const string Scripts;
	static const string Palettes;

	static const TFilePath SandboxProjectName;

	// TProject
	TProject();
	~TProject();

	TFilePath getName() const { return m_name; }
	TFilePath getProjectPath() const { return m_path; }
	TFilePath getProjectFolder() const { return m_path.getParentDir(); }

	void setFolder(string name, TFilePath path);
	void setFolder(string name);

	int getFolderCount() const;
	string getFolderName(int index) const;
	int getFolderIndex(string folderName) const;

	bool isConstantFolder(int index) const;

	TFilePath getFolder(int index) const;
	TFilePath getFolder(string name) const;

	TFilePath getScenesPath() const;

	TFilePath decode(TFilePath fp) const;

	bool isCurrent() const;

	void setSceneProperties(const TSceneProperties &sprop);
	const TSceneProperties &getSceneProperties() const
	{
		return *m_sprop;
	}

	//?????????????????????????????????????????????
	void setUseScenePath(string folderName, bool on);
	//?????????????????????????????????????????????
	bool getUseScenePath(string folderName) const;

	// nei due metodi seguenti fp e' un path assoluto (possibilmente con $scenepath)
	//????????????????????????????????????????????????
	int getFolderIndexFromPath(const TFilePath &fp);
	wstring getFolderNameFromPath(const TFilePath &fp);

	bool save(const TFilePath &projectPath);
	bool save();
	void load(const TFilePath &projectPath);

	static bool isAProjectPath(const TFilePath &fp);

private:
	// not implemented
	TProject(const TProject &src);
	TProject &operator=(const TProject &);
};

#ifdef WIN32
template class DVAPI TSmartPointerT<TProject>;
#endif
typedef TSmartPointerT<TProject> TProjectP;

//===================================================================

class DVAPI TProjectManager
{ // singleton
public:
	class Listener
	{
	public:
		virtual void onProjectSwitched() = 0;
		virtual void onProjectChanged() = 0;
		virtual ~Listener() {}
	};

private:
	std::vector<TFilePath> m_projectsRoots;
	std::vector<TFilePath> m_svnProjectsRoots;
	std::set<Listener *> m_listeners;

	void addDefaultProjectsRoot();

	TProjectManager();
	void notifyListeners();

	bool m_tabMode;
	bool m_tabKidsMode;

public:
	~TProjectManager();

	void notifyProjectChanged();

	static TProjectManager *instance();

	TFilePath getCurrentProjectPath();
	void setCurrentProjectPath(const TFilePath &fp);
	TProjectP getCurrentProject();

	void initializeScene(ToonzScene *scene);

	void saveTemplate(ToonzScene *scene);

	void addProjectsRoot(const TFilePath &fp);
	void addSVNProjectsRoot(const TFilePath &fp);

	//! returns the project root of the current project (if this fails, then returns the first project root)
	TFilePath getCurrentProjectRoot();

	TFilePath projectPathToProjectName(const TFilePath &projectPath);
	TFilePath projectNameToProjectPath(const TFilePath &projectName);
	TFilePath projectFolderToProjectPath(const TFilePath &projectFolder);
	TFilePath getProjectPathByName(const TFilePath &projectName);

	TProjectP loadSceneProject(const TFilePath &scenePath);
	void getFolderNames(std::vector<string> &names);

	void addListener(Listener *listener);
	void removeListener(Listener *listener);

	TProjectP createStandardProject();
	void createSandboxIfNeeded();

	bool isTabKidsModeEnabled() const { return m_tabKidsMode; }
	void enableTabKidsMode(bool tabKidsMode) { m_tabKidsMode = tabKidsMode; }

	bool isTabModeEnabled() const { return m_tabMode; }
	void enableTabMode(bool tabMode) { m_tabMode = tabMode; }

	TFilePath getSandboxProjectFolder();
	TFilePath getSandboxProjectPath();

	void getProjectRoots(std::vector<TFilePath> &projectRoots) const { projectRoots = m_projectsRoots; }

	bool isProject(const TFilePath &projectFolder);
};

#endif
