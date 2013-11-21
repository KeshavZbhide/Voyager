#ifndef KAMINO_SILENT_FACEBOOK_POST_H
#define KAMINO_SILENT_FACEBOOK_POST_H

#include "content\public\browser\web_contents.h"
#include "content\public\browser\notification_registrar.h"
#include "content\public\browser\notification_observer.h"
#include "KaminoBrowserContext.h"

class KaminoSilentFacebookPost: public content::NotificationObserver{
public:
	
	KaminoSilentFacebookPost();
	~KaminoSilentFacebookPost();
	void Start();
	void Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) OVERRIDE;
private:
	scoped_ptr<content::WebContents> web_content;
	scoped_ptr<content::KaminoBrowserContext> browser_context;
	void PostComment();
	void ExitFaceBookPostProcess();
	content::NotificationRegistrar registrar;

};

#endif