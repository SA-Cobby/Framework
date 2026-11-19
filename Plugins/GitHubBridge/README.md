# GitHub Bridge for Unreal Engine 5.4

Windows editor plugin for working with an existing Git checkout hosted on GitHub. Includes a dockable configuration window and an editor-close dialog. This is an editor tool, not a replacement for Unreal's source-control provider or a merge/conflict editor.

## Install

1. Close Unreal. Copy the `GitHubBridge` folder into `<YourProject>/Plugins/`.
2. Open the project in UE **5.4.4**, enable **GitHub Bridge** under Edit > Plugins if necessary, and restart.
3. Open **Window > GitHub Bridge**.

The packaged Windows DLL is built against UE 5.4.4. For another 5.4 patch or a source engine build, rebuild the plugin with that engine. Source is included.

## Configure

**Automatically connect Unreal's Git revision control** is enabled by default in the GitHub Bridge window. After successful project repository detection, it selects the engine's **Git** provider and refreshes project file status in the background. This enables Unreal's normal asset revision-control menus and status icons alongside GitHub Bridge's commit/push workflow. Selecting Git replaces any other active revision-control provider.

Uncheck this setting to stop GitHub Bridge from selecting/configuring Unreal's provider. It leaves the currently selected provider connected; disconnect or choose another provider through Unreal's own Revision Control menu. The preference is saved per project as `AutoConnectRevisionControl` in `Saved/Config/GitHubBridge.ini`.

The plugin prepares the Git binary path before the built-in Git provider loads, including a project-local `git` installation. If you change that path or enable automation after the provider has already loaded a different path, the settings panel requests an editor restart. It respects Unreal's existing global/per-project revision-control settings scope without changing that preference; if you have chosen global settings, provider/path changes use that shared scope. Connection status and errors appear below the checkbox. Connecting revision control opens the local checkout; it does not authenticate to GitHub, fetch, push, change branches, or initialize a repository.

The plugin checks for a project-local Git installation under the folder named `git` beside your `.uproject` on startup and whenever you click **Detect project repository**. Search order is `git/cmd/git.exe`, `git/bin/git.exe`, `git/git.exe`, then `git/mingw64/bin/git.exe`. The first executable found populates the Git executable field and is used for subsequent operations. If none is found, the configured executable path is retained (initially the standard system installation). You can still edit the field manually between detections.

Place a complete Git for Windows/PortableGit distribution there, keeping its supporting directories intact; copying only `git.exe` is insufficient. This is an optional local installation, separate from Git's hidden `.git` repository metadata folder. Add `/git/` to your repository's `.gitignore` if you do not want to commit the bundled Git installation. A broken local installation reports a Git error; fix/remove it or choose a working executable manually. Git itself is not bundled in this download.

On editor startup, the plugin automatically detects the nearest `.git` directory in the project folder or a parent folder. Git worktrees and submodules using a `.git` file are supported too. Git verifies the checkout and supplies its root, current branch, GitHub `origin` URL, and configured author name/email. This refreshes the displayed local settings without changing the repository. The Git executable path, GitHub username, and exit-reminder preference are retained.

Use **Detect project repository** to refresh these fields after an external branch switch or repository change, or after correcting the Git executable path. If no valid checkout is found, existing settings are kept. A missing/unsupported origin leaves the clone URL empty; detached HEAD leaves the branch empty with an explanation. Credentials embedded in an existing remote URL are never imported. Detection runs locally in the background and does not fetch, sign in, initialize a repository, or modify Git configuration.

- Install Git for Windows, including a current **Git Credential Manager (GCM)**. Enter the absolute path to `git.exe` if it differs from the default.
- Clone your GitHub repository outside Unreal first, or initialize the project with Git and create a matching GitHub repository. Select its **repository root**, which can contain the project in a subfolder. This plugin does not clone repositories or create GitHub repositories.
- Enter `https://github.com/OWNER/REPOSITORY.git` or `git@github.com:OWNER/REPOSITORY.git`, the working branch, GitHub username, and commit author name/email. GitHub's private noreply email can be used.
- **Apply origin + author settings** changes the checkout's `origin` URL and local `user.name`/`user.email`. Other users of that checkout will see these changes. It does not switch branches.
- For HTTPS, **Sign in to GitHub (browser)** delegates login to GCM. Complete the browser flow. GitHub account passwords cannot authenticate Git HTTPS. Older Git/GCM installations without `github login` must be updated. Existing credentials configured outside the plugin also work.
- For SSH, configure keys, agent, and GitHub host verification outside Unreal before use. The browser sign-in button is for HTTPS only.
- Set your desired existing branch and use **Fetch + switch branch**, then refresh status. Git can automatically create a tracking branch if exactly one matching remote branch exists. The plugin does not create arbitrary new branches.

Configuration is local to the Unreal project at `Saved/Config/GitHubBridge.ini`. No tokens or passwords are requested or saved by this plugin. HTTPS credentials are managed by GCM/the operating system; SSH keys remain in your existing SSH setup. Settings include account names, email, local paths, and clone URL, so exclude `Saved/` from Git.

## Daily use

Open **Window > GitHub Bridge Diff**, or click **Review changes / diff** in GitHub Bridge. **Pending changes vs last commit** shows the combined staged/unstaged diff against `HEAD` and previews untracked files as additions. **Latest commit vs previous commit** compares `HEAD` to its first parent; an initial commit is compared against an empty tree. Use the buttons to refresh after editing or saving. Unsaved Unreal assets are not represented until saved to disk.

The diff is read-only and uses a monospaced unified patch: `-` lines are removed and `+` lines are added. Binary files, including most `.uasset` and `.umap` assets, are identified without a text patch. Use Unreal's native asset Diff against Depot for supported asset types. Ignored untracked files are excluded. External Git diff/text-conversion helpers are disabled for this preview. Large previews are capped at approximately two million characters and fifty untracked files; use a Git client to inspect the full diff when a truncation notice appears. Reviewing does not stage, commit, save assets, or access GitHub.

All push actions now check the destination branch, fetch its latest history, and rebase local commits onto it when necessary before pushing. This includes native Unreal submissions, the plugin's commit/push button, exit-time push, and **Rebase + push existing commits**. If the destination branch does not exist yet, it is created without a rebase. If the fetched remote tip is already an ancestor of the local tip, no history rewrite is needed.

Before a required rebase, tracked files must be clean. Commit or stash unselected tracked edits yourself; the plugin does not automatically stash or change their staging state. Untracked files are left alone, and Git will refuse a rebase that would overwrite them. An existing merge/rebase/cherry-pick must be finished or aborted first. A queued native submission is also refused if the branch has advanced since it was queued; review the branch and use the existing-commits action to retry.

The plugin preserves the pre-rebase tip under `refs/github-bridge/pre-rebase/<id>` and reports the ref in its output. Rebase conflicts stop the push and retain Git's conflict state for resolution with your usual Git tools (`git rebase --continue` or `git rebase --abort`). No force push is used. If a push is rejected after a successful rebase, the rebased local history and backup remain available. Restart Unreal before editing assets changed on disk by the rebase. GitHub account-backed tests are separate from the included local-remote tests.

**Push to GitHub after a successful Unreal Git submit** is enabled by default. It applies to the built-in Git provider's file-based submit dialog, including Submit Content and asset submission actions that use that dialog. Unreal still handles file selection, saving, validation, and marking new files for add. GitHub Bridge preserves pre-submit and asset virtualization processing, executes the native Git CheckIn for the selected files, then queues an asynchronous push only if that check-in succeeds. Failed/cancelled submits do not queue a push. Other revision-control providers are unaffected.

The queued push synchronizes and rebases the submitted local tip when necessary, then uploads the resulting history to the configured GitHub branch. It never stages or commits other changes. A notification reports success or failure; detailed output appears in GitHub Bridge. If authentication, branch checks, or the network prevent the push, the local commit remains safe. Use **Rebase + push existing commits** to retry without staging any additional files. GitHub Bridge's own commit/push action remains separate and does not trigger a second automatic push.

Disable the checkbox to return native submission to its usual local-commit behavior. Its per-project key is `PushAfterUnrealSubmit` in the plugin settings ini. This uses UE 5.4's public submit override; if another extension owns that override, GitHub Bridge leaves it alone and reports that automatic submit push is unavailable. It does not watch arbitrary external commits, replace Git hooks, or change engine files. The native check-in is refused while a GitHub Bridge operation is running; wait for the current operation before submitting. Unreal may already have marked newly selected files for add before reaching this check.

**Refresh status** shows changed, deleted, staged, and untracked files. Review this before committing. **Save assets, commit ALL changes + push** saves dirty Unreal packages, stages all non-ignored changes across the **whole repository**, commits with your message, and pushes to the explicitly configured branch. Existing staged changes are included. This version does not have per-file selection. A sensible `.gitignore` is required; configure Git LFS for Unreal binary assets before the first commit.

**Pull** requires a clean working tree and uses fast-forward only. Diverged histories are refused. **Switch branch** also requires a clean tree. Both can change assets on disk: close asset editors first and restart Unreal afterwards so loaded assets do not remain stale. Resolve conflicts, stash, create branches, and manage LFS locks using your usual Git tools.

When you exit normally through the editor window or File > Exit, GitHub Bridge first checks Unreal's unsaved map/content packages, the repository's staged/modified/deleted/untracked files, and local commits not present on the configured `origin` branch. If everything is saved, clean, and already pushed, the plugin's exit dialog is skipped and Unreal closes through its normal flow. A clean branch that is only behind the remote also has nothing to push.

The commit comparison uses local remote-tracking information, updated by normal fetch/push operations; exit does not contact GitHub or require a login. If no remote-tracking branch exists, existing local commits are treated as needing a push. If Git/configuration errors prevent verification, the dialog remains available and explains the uncertainty. Ignored files do not trigger a Git-change prompt. Unsaved Unreal assets trigger it even when the on-disk repository is currently clean.

When there is work to save/push or its status cannot be verified, the plugin offers:

- **Save, commit + push, then close**: save packages first, stage/commit non-ignored changes if any, and push. With a clean tree, this still pushes existing local commits. Missing configuration, cancelled saves, rejected commits, dirty post-hook state, timeouts, and failed pushes keep Unreal open.
- **Close without pushing**: continue Unreal's normal shutdown/save prompts.
- **Cancel exit** (or the dialog's X): keep Unreal open.

Turn off the exit reminder in the plugin window if desired. The close hook does not cover crashes, forced process termination, operating-system shutdown, or commandlets. Other editor systems can still cancel closure after a successful push. A successful push cannot be undone by subsequently cancelling an unrelated close prompt.

Network work runs off the UI thread with a three-minute timeout per Git command. A timed-out command is terminated along with its child processes. If a remote accepted a push just before a timeout, refresh/check GitHub before retrying. Closing the exit dialog while Git is running cancels the exit but allows the current Git operation to finish. A normal exit is blocked while Git is busy.

Push never uses force. A rejected push retains the local commit: reconcile the branch externally and retry. Failed commits can leave files staged; no reset or cleanup discards your changes. Git hooks and configured LFS filters run normally and must work without terminal input. Only GitHub.com clone URLs are supported in this version, not GitHub Enterprise/custom hosts.

## Build and test

```powershell
& 'D:\Engines\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin `
  -Plugin='G:\Assistant\GitHubBridge\GitHubBridge.uplugin' `
  -Package='G:\Assistant\GitHubBridgeBuild' -TargetPlatforms=Win64
```

Adjust paths for your machine. Run the editor automation test `GitHubBridge.RepositoryGuardsAndProcessArguments` in Session Frontend > Automation. It creates an isolated repository under the project's `Saved/Automation/GitHubBridge` folder and needs Git at the default Windows location. It verifies actual process argument handling, local repository configuration, branch/remote guards, clean-tree requirements, and token-in-URL rejection without accessing GitHub.

Real-account browser authentication and live GitHub pushes require a separate account-backed manual check. No GitHub credentials or remote repository are bundled.

The `GitHubBridge.UnrealGitProviderIntegration` automation test exercises startup provider selection, the engine's actual Git provider, opt-out behavior, and a Connect/status refresh. Run it only from an isolated test project that is already a Git checkout. It temporarily selects Git and restores the original provider afterwards; it does not initialize or commit a repository. Automatic revision-control connection runs in both normal and unattended editors and is skipped for commandlets.

