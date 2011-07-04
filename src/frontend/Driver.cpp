#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Driver/Arg.h>
#include <clang/Driver/ArgList.h>
#include <clang/Driver/CC1Options.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/DriverDiagnostic.h>
#include <clang/Driver/OptTable.h>
#include <clang/Driver/Option.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/DiagnosticOptions.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/FrontendTool/Utils.h>
#include <llvm/LLVMContext.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Timer.h>
#include <llvm/Support/Path.h>
#include <llvm/Target/TargetSelect.h>

using namespace clang;

llvm::sys::Path GetExecutablePath(const char *Argv0, bool CanonicalPrefixes) {
  if (!CanonicalPrefixes)
    return llvm::sys::Path(Argv0);

  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *P = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::Path::GetMainExecutable(Argv0, P);
}

int main(int argc, char* argv[])
{
  argc = argc-1;
  argv = argv+1;
  llvm::OwningPtr<CompilerInstance> clang(new CompilerInstance());

  // clang->setLLVMContext(new llvm::LLVMContext());

  // Initialize targets first, so that --version shows registered targets.
  // llvm::InitializeAllTargets();
  // llvm::InitializeAllAsmPrinters();
  // llvm::InitializeAllAsmParsers();

  TextDiagnosticBuffer *diagsBuffer = new TextDiagnosticBuffer;
  llvm::IntrusiveRefCntPtr<DiagnosticIDs> diagIDs(clang->getDiagnostics().getDiagnosticIDs());
  Diagnostic diags(diagIDs, diagsBuffer);
  CompilerInvocation::CreateFromArgs(clang->getInvocation(),
      const_cast<const char**>(argv),
      const_cast<const char**>(argv+argc),
      diags);

  if(clang->getHeaderSearchOpts().UseBuiltinIncludes &&
      clang->getHeaderSearchOpts().ResourceDir.empty())
    clang->getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(argv[0], (void*)(intptr_t)GetExecutablePath);

  clang->createDiagnostics(argc, argv);
  if(!clang->hasDiagnostics())
    return 1;

  // llvm::install_fatal_error_handler

  diagsBuffer->FlushDiagnostics(clang->getDiagnostics());

  bool success = ExecuteCompilerInvocation(clang.get());

  llvm::TimerGroup::printAll(llvm::errs());
  // llvm::remove_fatal_error_handler();

  llvm::llvm_shutdown();

  return !success;
}
