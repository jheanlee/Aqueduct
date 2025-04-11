import { Box, Container, Separator } from "@chakra-ui/react";
import Navbar from "./navbar.tsx";
import { Outlet } from "react-router";

function Layout() {
  return (
    <Container w="100vw" h="100vh" m="0" p="0" fluid>
      <Navbar />
      <Separator />
      <Box as="main" h="calc(100vh - 3.5rem)">
        <Outlet />
      </Box>
    </Container>
  );
}

export default Layout;
