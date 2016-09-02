# XRulez
"XRulez" is a Windows executable that can add malicious rules to Outlook from the command line of a compromised host.

To to learn more about the tool and the research behind this project, visit [https://labs.mwrinfosecurity.com/blog/malicious-outlook-rules](https://labs.mwrinfosecurity.com/blog/malicous-outlook-rules).

## Description
Outlook rules can be used to achieve persistence on Windows hosts by creating a rule that executes a malicious payload. The rule can be setup to execute when the target receives an email with a specific keyword in the subject. An attacker can then drop shells on a target as and when they require by simply sending an email.

In the past, this technique could only be done via the Outlook GUI. However, XRulez achieves this from the command line.

XRulez is open source software, maintained by [MWR InfoSecurity](https://www.mwrinfosecurity.com/).

## Getting Started
### Prerequisites
- The target machine has already been compromised.
- The target has an authenticated session with the exchange server.
- The target can access a file-share that we can write to.
- We can send an email to the targets exchange account.

### Usage
From a compromised system:

1. Upload XRulez.exe to the target.

2. Run XRulez.exe with '-l' to display a list of MAPI profiles installed on the system.

		XRulez.exe -l
3. Run XRulez.exe with '-a' and the required parameters to add a new rule.

		XRulez.exe -a [--profile PROFILE] [--name NAME] [--trigger TRIGGER] [--payload PAYLOAD]

4. Send an email to the target with the previously set keyword in the subject field.
5. Wait for shell.

XRulez takes four required parameters when adding a new rule. 

- Profile Name: Outlook MAPI Profile, installed profiles can be found using '-l'.
- Rule Name:  A rule description, e.g SPAM Filter.
- Rule Trigger: Keyword that will be searched for in the subject field of all incoming messages.
- Payload Path: Path to the application that will be executed when the condition is met.

> XRulez scans %APPDATA%\Microsoft\Outlook\ for .xml files that represent Outlook profiles. It also provides the last modified time and a suggestion for the profile to select based on the modification times. However, the .xml files are only updated when Outlook is closed. This could cause confusion if the user has just swapped accounts. Most users however will have only one default profile, called "Outlook".
> 
> After the rule has been created, XRulez can be removed from the target system and is no longer needed.
> 
> The payload application can be anything that is opened normally by ShellExec. This includes .exe, .bat, .vbs. It doesn't include .ps1 files, as by default they're opened in the default text editor.
> 
> Since MAPI is architecture dependent, different versions will be required for 32-bit / 64-bit systems. Ensure that you're using the correct version before you run XRulez.exe. If you fail to do so, a popup box will be displayed to the user stating that "MAPI can't find an Outlook client".

### Limitations
Outlook must be open on the target machine. If not, there will be no session to share and therefore no connection to Exchange.

Outlook uses ShellExec to open the payload application which means that the payload can't be executed with arguments, requiring the payload to be an all enclosed application hosted on the disk or externally.
## Compiling
In order to compile from source, you'll need to include / link the following headers and libraries.
- EdkGuid.h and EdkMdb.h, available to download from /src
- Outlook 2010 Header Files, available to download from [https://www.microsoft.com/en-gb/download/details.aspx?id=12905](https://www.microsoft.com/en-gb/download/details.aspx?id=12905)
- MAPI Stub Library, available to download from [https://mapistublibrary.codeplex.com/](https://mapistublibrary.codeplex.com/)

## License
XRulez is released under a 3-clause BSD License. See LICENSE for full details.
