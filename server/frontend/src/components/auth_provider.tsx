import { ReactNode, useEffect, useState } from "react";
import { useNavigate } from "react-router";
import {cronFetcher, fetcher} from "../core/fetcher.ts";
import { Buffer } from "buffer";
import {toaster} from "./ui/toaster.tsx";

interface AuthProviderProps {
  children: ReactNode;
}

function AuthProvider({ children }: AuthProviderProps) {
  const [isInitialised, setIsInitialised] = useState<boolean>(false);

  const navigate = useNavigate();

  useEffect(() => {
    if (isInitialised) {
      return;
    }

    const access_token = localStorage.getItem("aqueduct.access_token");

    //  no token
    if (access_token === null) {
      navigate({ pathname: "/login" });
      setIsInitialised(true);
      return;
    }

    //  time verification
    if (JSON.parse(Buffer.from(access_token.split('.')[1], "base64").toString("ascii")).exp + 300 < Date.now() / 1000) {
      toaster.create({
        description: "Session expired",
        type: "info"
      })
      navigate({ pathname: "/login" });
      setIsInitialised(true);
      return;
    }

    //  set token
    fetcher.defaults.headers['Authorization'] = access_token;
    cronFetcher.defaults.headers['Authorization'] = access_token;

    setIsInitialised(true);
  }, []);
  return <>{children}</>;
}

export default AuthProvider;