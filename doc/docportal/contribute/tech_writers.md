# Contribution guide for technical writers
The user documentation is located at [docs.scummvm.org](https://docs.scummvm.org/en/latest/) . It is written in reStructuredText or markdown, built using Sphinx, and deployed by Read the Docs. The recommonmark extension is used to enable parsing of markdown files. The [sphinx-panels](https://sphinx-panels.readthedocs.io/en/latest/) extension is used to provide Bootstrap functionality by using custom rST directives. All documentation is written to conform to the Google developer documentation [style guide](https://developers.google.com/style).  

There are two ways to contribute to documentation; working online directly on Github, or by forking and cloning the scummvm repository to work locally.

## Small changes to existing documentation

This may include correcting a mistake, or fixing spelling or grammar. 

The easiest way to make a change is to use the **Edit this page on GitHub** link on the applicable page to edit documentation online, on GitHub. This method does not require you to fork the repository, but do bear in mind that you won’t be able to upload images while working in this way. 

Once you are happy with your changes, create a pull request (PR). The ScummVM team will review the pull request, and if there are no issues, it will be merged into the existing code. 

## Large changes, new or existing documentation

This may include documenting a new feature, documenting a new or existing port, or writing a guide. 

### Set up your environment

* Install Python3: 
	* Linux: Comes with Python3 installed. 
	* macOS: See [this tutorial](https://www.digitalocean.com/community/tutorials/how-to-install-python-3-and-set-up-a-local-programming-environment-on-macos) .
	* Windows: See [this tutorial](https://phoenixnap.com/kb/how-to-install-python-3-windows).
* Install pip3: 
	* Linux: Use `sudo apt install python3-pip`. 
	* Windows: Pip3 is installed when you install Python3. 
	* macOS: Pip3 is installed if you install Python3 with Homebrew. 
* Use pip3 to install [sphinx](https://www.sphinx-doc.org/en/master/usage/installation.html), [sphinx-panels](https://sphinx-panels.readthedocs.io/en/latest/#installation) and [recommonmark](https://www.sphinx-doc.org/en/master/usage/markdown.html).   
* [Install Git](https://docs.github.com/en/free-pro-team@latest/github/getting-started-with-github/set-up-git), if you don’t have it already. 
* [Fork and clone](https://docs.github.com/en/free-pro-team@latest/github/getting-started-with-github/fork-a-repo) the [scummvm repository](https://github.com/scummvm/scummvm). 
* [Create and checkout](https://docs.github.com/en/free-pro-team@latest/github/collaborating-with-issues-and-pull-requests/creating-and-deleting-branches-within-your-repository) a new branch.

### Create and edit documentation

Documentation files are found in `scummvm/doc/docportal/`
Images, screenshots and GIFs are found in `scummvm/doc/docportal/images/`. 

There are many options for editing and creating documents. A particularly good one is [VS Code](https://code.visualstudio.com), which features extensions to help you write in reStructuredText/markdown, as well as featuring Git integration.  

Here is a [handy guide](https://rest-sphinx-memo.readthedocs.io/en/latest/intro.html) to reStructuredText as it is used by Sphinx. 

If you are creating a new Platform or Settings page, use the template provided in the docportal directory: `platform_template.rst` or `settings_template.rst`. This is to ensure our documentation remains consistent and predictable for the end user. 

### Preview your documentation

To build a preview of the documentation, open a new Terminal window, and change your working directory until you are in the `docportal` directory. Use the `make html` command. The output HTML files will be in the `scummvm/doc/docportal/_build/html/` directory.  Open the `index.html` file in a browser to view the index page of the site. 

Note:
Do not commit the `_build` folder to your branch. The documentation is built by Read the Docs before deployment, so the local build is not required. 

### Commit your work

Follow the ScummVM [commit guidelines](https://wiki.scummvm.org/index.php/Commit_Guidelines) when you [make a commit](https://github.com/git-guides/git-commit) .

### Create a Pull Request

For documentation related to the CURRENT release, create a pull request against the branch for the current version. For example, to make a change or add documentation applicable to ScummVM version 2.2.0, open a pull request against `branch-2-2-0`.  For documentation related to the NEXT release, open a pull request against the `master` branch. 
