# Validation

## Version 0.8.0 diff viewer and rebase workflow — 17 November 2026

- UE **5.4.4** Win64 packaging succeeded. All seven Unreal automation tests passed (7 succeeded, 0 failed).
- `GitHubBridge.ReadOnlyCommitDiff` verifies initial/unborn repositories, staged and unstaged edits, untracked additions, deletions, binary identification, ignored-file exclusion, root-commit comparison, and latest-commit/first-parent comparison. It asserts that viewing diffs leaves status and the index unchanged and succeeds with an invalid external-diff command configured, confirming that external helpers are not invoked.
- The updated local-remote test verifies successful divergent rebasing and upload of the rewritten tip, preservation of untracked files and pre-rebase backup refs, refusal with dirty tracked files, server-hook rejection, and a real content conflict. It confirms no remote change on conflict and that aborting restores the original local tip.
- Native submission, repository detection/guards, conditional exit prompting, unsaved packages, and provider startup/connection tests also passed. All transport tests use a disposable local bare repository; no GitHub account/network operation was performed.
- The diff tab and changed notification text compiled successfully; interactive layout review remains manual. Binary Unreal assets receive a change notice rather than a text patch. Asset refresh after a rebase still requires an editor restart before further editing.

## Version 0.6.0 conditional exit prompt

- UE **5.4.4** Win64 packaging succeeded. All six Unreal automation tests passed (6 succeeded, 0 failed).
- The real-Git fixture now checks exit decisions for an empty unborn repository, unpublished local history, a clean tree after a successful push, untracked files, tracked modifications, staged changes, a clean branch that is only behind its destination, divergent local history, and unverifiable branch configuration.
- `GitHubBridge.UnsavedAssetsRequireExitPrompt` creates a dirty project package, verifies that Unreal's normal dirty-package query finds it, and checks that unsaved assets require a prompt even when Git is otherwise clean and synced. The test clears its dirty flag without writing an asset.
- The close hook uses this decision and the same dirty-package query as Unreal's save flow. Saving remains before commit/push. No remote fetch or GitHub access occurs during the exit check; local remote-tracking references determine pending commits.
- Interactive exit-dialog verification remains manual; the existing save cancellation and push-failure handling is retained.

## Version 0.5.0 push after native Unreal submission

- UE **5.4.4** Win64 packaging succeeded. All five Unreal automation tests passed (5 succeeded, 0 failed).
- `GitHubBridge.NativeSubmitSelectedFiles` exercised the registered public submit override and the real engine Git CheckIn operation. It verified selected-file submission, description preservation, one success callback, no callback on failed check-in, opt-out behavior, and that another extension's submit override is not replaced.
- `GitHubBridge.PushOnlyPreservesUncommittedFiles` pushed a real immutable commit to an isolated local bare repository through a local SSH replacement, verified the remote commit ID, and verified that an unselected file stayed untracked. It then advanced the remote through a competing checkout and confirmed a rejected push retained both the local commit and unrelated edits. No connection to GitHub was made.
- Existing repository, project-local Git discovery, and native provider startup/connection tests also passed.
- The standard Git file-submit integration preserves pre-submit and asset virtualization handling. Virtualization-enabled binary asset submission and interactive submit-dialog visuals still need a project-backed manual check. Live GitHub credentials/network behavior remain outside the automated tests.

## Version 0.4.0 Unreal revision-control integration

- UE **5.4.4** Win64 packaging succeeded, including the minimal PreDefault bootstrap module and PostEngineInit editor module.
- All three Unreal automation tests passed (3 succeeded, 0 failed). The run exercised the actual startup auto-connection path: the log recorded early Git-path preparation and a successful native Git Connect/status refresh before the tests started.
- `GitHubBridge.UnrealGitProviderIntegration` asserts startup selection of Git, the provider's repository root, availability, the real Connect operation, and that disabling automation leaves provider selection and binary settings unchanged. It runs only inside an isolated Git-backed test project; no Framework repository settings were changed during testing.
- GUI verification remains incomplete: normal editor smoke runs stalled before plugin UI initialization. The unattended editor completed startup and the full test suite. No account-backed GitHub operation was performed.
- Changing the native provider's cached Git executable after startup can require a restart, which is reported in the plugin panel. Disabling automation does not disconnect a provider selected through Unreal itself.

## Version 0.3.0 project-local Git discovery

- UE **5.4.4** Win64 packaging succeeded. Both Unreal automation tests passed (2 succeeded, 0 failed).
- Added `GitHubBridge.ProjectLocalGitDiscovery` to check all four supported installation layouts, deterministic preference for the `cmd` launcher, paths containing spaces, and fallback when the project has no local executable or only an empty `git` folder. These are location-selection tests; placeholder executable files are never run.
- The existing repository/process test still exercises the actual installed Git executable. A complete project-local PortableGit installation has not been exercised end to end.

## Version 0.2.0 repository detection

- Rebuilt successfully against Unreal Engine **5.4.4** for Win64.
- The expanded `GitHubBridge.RepositoryGuardsAndProcessArguments` test passed in Unreal's automation runner.
- Added real-Git automation coverage for detecting a repository from a nested project folder, importing origin/current branch/author identity, retaining the GitHub account setting, detecting a linked worktree through its `.git` file, reporting detached HEAD without guessing a branch, clearing absent/unsupported origins, excluding credential-bearing URLs, and preserving configuration on invalid repository metadata.
- Detection uses local reads only. It runs in the background at editor startup and from the **Detect project repository** button.

## Original version validation

- Packaged successfully with Unreal Engine **5.4.4**, Win64 Development Editor, using Unreal AutomationTool `BuildPlugin`.
- `GitHubBridge.RepositoryGuardsAndProcessArguments` passed inside Unreal Editor's automation runner (exit code 0).
- The test uses the real Git executable and isolated local fixtures. It covers configuration, unborn branches, quoted commit messages, paths containing spaces, dirty-tree pull/switch refusal, empty commit message refusal, branch mismatch, separate push URL refusal, credential-bearing URL refusal, invalid branches, and subfolder-root refusal.
- A separate editor instance loaded the plugin test project. Visual verification could not be completed because the computer-use screenshot service timed out on both attempts. The settings layout, modal dialog behavior, and all exit choices still require a manual UI check.
- Real GitHub browser sign-in and remote pull/push were not tested. No GitHub repository was modified. The installed Git Credential Manager is older than the browser-login command this plugin requires; use a current Git for Windows/GCM installation or existing SSH authentication.

Suggested account-backed smoke test: use a disposable GitHub repository, configure origin/identity/branch, sign in, modify a project file, inspect status, commit/push, and pull a separate remote commit. Verify that cancelling exit keeps Unreal open, closing without push follows Unreal's save flow, and a rejected push keeps the exit dialog open while retaining the local commit. Restart Unreal after pulling/switching asset revisions.
