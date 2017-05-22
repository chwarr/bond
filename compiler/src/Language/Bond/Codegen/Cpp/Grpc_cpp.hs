-- Copyright (c) Microsoft. All rights reserved.
-- Licensed under the MIT license. See LICENSE file in the project root for full license information.

{-# LANGUAGE QuasiQuotes, OverloadedStrings, RecordWildCards #-}

module Language.Bond.Codegen.Cpp.Grpc_cpp (grpc_cpp) where

import Data.Monoid
import Prelude
import qualified Data.Text.Lazy as L
import Text.Shakespeare.Text
import Language.Bond.Syntax.Types
import Language.Bond.Codegen.TypeMapping
import Language.Bond.Codegen.Util
import qualified Language.Bond.Codegen.Cpp.Util as CPP

-- | Codegen template for generating /base_name/_grpc.cpp containing
-- definitions of helper functions and schema metadata static variables.
grpc_cpp :: MappingContext -> String -> [Import] -> [Declaration] -> (String, L.Text)
grpc_cpp cpp file _imports declarations = ("_grpc.cpp", [lt|
#include "#{file}_grpc.h"

#{CPP.openNamespace cpp}
#{doubleLineSep 1 grpc declarations}
#{CPP.closeNamespace cpp}

|])
  where
    idl = MappingContext idlTypeMapping [] [] []

    grpc s@Service {..} = [lt|
const char* #{declName}::method_names[] =
{
    #{newlineSep 1 methodStrings serviceMethods}
};
|]
      where
        methodStrings Function{..} = [lt|"/#{getDeclTypeName idl s}/#{methodName}",|]
        methodStrings Event{..} = [lt|"/#{getDeclTypeName idl s}/#{methodName}",|]

    grpc _ = mempty
