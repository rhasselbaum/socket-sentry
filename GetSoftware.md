If you're running a recent version of [Kubuntu](http://www.kubuntu.org) or [openSUSE](http://www.opensuse.org/en), you will probably want to install the binary packages. If not, you can build and install the project from the source code. Don't worry, it's not hard once you have the prerequisites. One important thing to note no matter how you install the software is that you must **restart Plasma by logging out and logging back in** before using the widget. Otherwise, you will probably get an error. Now let's get started!



# Installing the binary packages #

Installation packages and repositories are available for **Kubuntu** and **openSUSE**. For Linux distributions or versions other than those below, follow the instructions to [build and install from the source code](#Building_from_sources.md).

## Kubuntu ##

To install on Kubuntu, first add the package repository to your software sources list. To do this, launch **KPackageKit** and go to **Settings > Edit Software Sources > Other Software > Add...** Then copy and paste the line below that matches your Kubuntu version into the text box.

| **Release** | **Line** |
|:------------|:---------|
| Kubuntu 10.04 (Lucid) | `deb http://download.opensuse.org/repositories/home:/rhasselbaum/xUbuntu_10.04  ./` |
| Kubuntu 9.10 (Karmic) | `deb http://download.opensuse.org/repositories/home:/rhasselbaum/xUbuntu_9.10  ./` |

Click **Close** and **Reload** when prompted. Then exit out of **KPackageKit**.
Next, install the GPG public key for the repository by opening and terminal and running the following command:

`wget -q https://socket-sentry.googlecode.com/files/pubkey.txt -O- | sudo apt-key add - && sudo apt-get update`

Now you are ready to install the package, which you can accomplish with:

`sudo apt-get install socketsentry`

**_Important:_ Please restart Plasma by logging out and logging back in before trying to use the widget. Otherwise, you will get an error.**

Now you are ready to run the widget on Kubuntu. Enjoy!

## openSUSE ##
To install on openSUSE, click the 1-Click install link below matching your version, then follow the prompts.

  * [openSUSE 11.3](http://software.opensuse.org/ymp/home:rhasselbaum/openSUSE_11.3/socketsentry.ymp)
  * [openSUSE 11.2](http://software.opensuse.org/ymp/home:rhasselbaum/openSUSE_11.2/socketsentry.ymp)

**_Important:_ Please restart Plasma by logging out and logging back in before trying to use the widget. Otherwise, you will get an error.**

Now you are ready to run the widget on openSUSE. Enjoy!

# Building from sources #

To build and install from the source code, download the latest source release from the [Downloads](http://code.google.com/p/socket-sentry/downloads/list) page and extract it to a folder of your choice. Next, follow the instructions in the **INSTALL** file located at the project root directory. You'll need to have at least version 4.3 of KDE running, as well as a few other prerequisites that are detailed in the file. Don't forget to restart Plasma as directed when you're done. And if you run into trouble, please don't hesitate to ask for help in our [discussion group](http://groups.google.com/group/socket-sentry).