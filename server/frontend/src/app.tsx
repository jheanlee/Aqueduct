import { HashRouter, Route, Routes } from "react-router";
import Client from "./pages/client.tsx";
import Login from "./pages/login.tsx";
import Layout from "./components/layout.tsx";
import Dashboard from "./pages/dashboard.tsx";
import Access from "./pages/access.tsx";
import { Toaster } from "./components/ui/toaster.tsx";
import AuthProvider from "./components/auth_provider.tsx";

function App() {
  return (
    <>
      <HashRouter>
        <AuthProvider>
          <Routes>
            <Route element={<Layout />}>
              <Route path="/" element={<Dashboard />} />
              <Route path="client" element={<Client />} />
              <Route path="login" element={<Login />} />
              <Route path="access" element={<Access />} />
            </Route>
          </Routes>
        </AuthProvider>
      </HashRouter>
      <Toaster />
    </>
  );
}

export default App;
